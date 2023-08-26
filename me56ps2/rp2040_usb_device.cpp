#include <algorithm>
#include <string.h>
#include <cctype>
#include <cstdint>

#include "hardware/regs/usb.h"
#include "hardware/structs/usb.h"
#include "hardware/irq.h"
#include "hardware/resets.h"

#include "usb_struct.h"
#include "rp2040_usb_device.h"

rp2040_usb_device *rp2040_usb_device::instance = nullptr;

void rp2040_usb_device::bus_reset() {
    this->printf("BUS_RESET\r\n");
    usb_hw->dev_addr_ctrl = 0;
    configured = false;
}

void rp2040_usb_device::dump_hex_and_ascii(const void *data, const size_t length)
{
    const uint8_t *c = reinterpret_cast<const uint8_t *>(data);
    for (size_t offset = 0; offset < length; offset += 16) {
        this->printf("  %04lx: ", offset);
        for (size_t p = 0; p < 16; p++) {
            if (offset + p < length) {
                this->printf("%02x ", c[offset + p]);
            } else {
                this->printf("   ");
            }
        }
        for (size_t p = 0; p < 16 && offset + p < length; p++) {
            this->printf("%c", isprint(c[offset + p]) ? c[offset + p] : '.');
        }
        this->printf("\r\n");
    }
}

void rp2040_usb_device::_irq_handler_usbctrl(void) {
    instance->irq_handler_usbctrl();
}

uint32_t rp2040_usb_device::get_ep_pid(const uint8_t ep_addr)
{
    const auto idx = get_usb_ep_index(ep_addr);
    const auto pid = ep_next_pid[idx];

    if (pid == DATA_PID::DATA0) {
        ep_next_pid[idx] = DATA_PID::DATA1;
        return USB_BUF_CTRL_DATA0_PID;
    } else {
        ep_next_pid[idx] = DATA_PID::DATA0;
        return USB_BUF_CTRL_DATA1_PID;
    }
}

void rp2040_usb_device::transmit(const uint8_t ep_addr, const void *data, const int len)
{
    this->printf("transmit ep_addr[0x%02x], length: %d\r\n", ep_addr, len);

    if (len > 0) {
        memcpy(get_usb_ep_buf_ptr(ep_addr), data, len);
        dump_hex_and_ascii(data, len);
    }

    const uint32_t val = USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL | get_ep_pid(ep_addr) | len;

    *(get_usb_ep_buf_ctrl_ptr(ep_addr)) = val;
}

void rp2040_usb_device::receive(const uint8_t ep_addr, const int max_len)
{
    uint32_t val = USB_BUF_CTRL_AVAIL | get_ep_pid(ep_addr) | max_len;

    *(get_usb_ep_buf_ctrl_ptr(ep_addr)) = val;
}

void rp2040_usb_device::handle_setup_packet(const volatile struct usb_setup_packet *_pkt)
{
    struct usb_setup_packet pkt;
    pkt.bmRequestType = _pkt->bmRequestType;
    pkt.bRequest = _pkt->bRequest;
    pkt.wValue = _pkt->wValue;
    pkt.wIndex = _pkt->wIndex;
    pkt.wLength = _pkt->wLength;
    last_setup_packet = pkt;
    this->printf("Setup packet\r\n");
    this->printf("  bmRequestType: 0x%02x\r\n", pkt.bmRequestType);
    this->printf("  bRequest: 0x%02x\r\n", pkt.bRequest);
    this->printf("  wValue: 0x%04x\r\n", pkt.wValue);
    this->printf("  wIndex: 0x%04x\r\n", pkt.wIndex);
    this->printf("  wLength: 0x%04x\r\n", pkt.wLength);

    ep_next_pid[get_usb_ep_index(USB_ENDPOINT_CONTROL_IN)] = DATA_PID::DATA1;

    const auto req_type = static_cast<USB_REQUEST_TYPE>(pkt.bmRequestType & USB_REQUEST_TYPE_BIT_MASK);
    const auto req = static_cast<USB_REQUEST>(pkt.bRequest);
    if (req_type == USB_REQUEST_TYPE_STANDARD && req == USB_REQUEST_SET_ADDRESS) {
        ep0_write(nullptr, 0);
        return;
    }

    const auto processed = setup_packet_callback(&pkt);
    if (!processed) {
        this->printf("ep0: Stall.\r\n");
        ep0_stall();
    }
}

void rp2040_usb_device::_ep0_in_transferred_callback(const void *data, const int len)
{
    instance->ep0_in_transferred_callback(data, len);
}

void rp2040_usb_device::ep0_in_transferred_callback(const void *data, const int len)
{
    const auto req_type = static_cast<USB_REQUEST_TYPE>(last_setup_packet.bmRequestType & USB_REQUEST_TYPE_BIT_MASK);
    const auto req = static_cast<USB_REQUEST>(last_setup_packet.bRequest);

    if (req_type == USB_REQUEST_TYPE_STANDARD && req == USB_REQUEST_SET_ADDRESS) {
        const int device_address = last_setup_packet.wValue & 0x7f;
        this->printf("Set address: %d\r\n", device_address);
        usb_hw->dev_addr_ctrl = device_address;
        return;
    }

    receive(USB_ENDPOINT_CONTROL_OUT, 64);
}

void rp2040_usb_device::handle_buff_status(void)
{
    const auto buf_status = usb_hw->buf_status;
    uint32_t bit = 1;
    for (int idx = 0; idx < 32; bit <<= 1, idx++) {
        if ((buf_status & bit) == 0) {continue;}
        const auto ep_addr = get_usb_ep_addr_by_index(idx);
        const auto buf = get_usb_ep_buf_ptr(ep_addr);
        const auto len = *(get_usb_ep_buf_ctrl_ptr(ep_addr)) & USB_BUF_CTRL_LEN_MASK;

        if (is_dir_out(ep_addr)) {
            this->printf("received ep_addr[0x%02x], length: %d\r\n", ep_addr, len);
            dump_hex_and_ascii(buf, len);
        }

        if (transferred_callback[idx] != nullptr) {
            transferred_callback[idx](buf, len);
        }

        hw_clear_alias(usb_hw)->buf_status = bit;
    }
}

void rp2040_usb_device::irq_handler_usbctrl(void)
{
    uint32_t ints = usb_hw->ints;

    if (ints & USB_INTS_BUS_RESET_BITS) {
        clear_sie_status(USB_SIE_STATUS_BUS_RESET_BITS);
        bus_reset();
    }

    if (ints & USB_INTS_SETUP_REQ_BITS) {
        clear_sie_status(USB_SIE_STATUS_SETUP_REC_BITS);
        handle_setup_packet(reinterpret_cast<volatile struct usb_setup_packet *>(&usb_dpram->setup_packet));
    }

    if (ints & USB_INTS_BUFF_STATUS_BITS) {
        handle_buff_status();
    }

    irq_clear(USBCTRL_IRQ);
}

int rp2040_usb_device::_dummy_printf(const char *fmt, ...)
{
    return 0;
}

rp2040_usb_device::rp2040_usb_device(int (*printf)(const char *fmt, ...))
{
    if (printf != nullptr) {
        this->printf = printf;
    } else {
        this->printf = _dummy_printf;
    }

    for (int i = 0; i < 32; i++) {
        transferred_callback[i] = nullptr;
    }
    transferred_callback[get_usb_ep_index(USB_ENDPOINT_CONTROL_IN)] = _ep0_in_transferred_callback;
    configured = false;
}

bool rp2040_usb_device::init(void)
{
    if (instance != nullptr) {
        return false;
    }
    instance = this;

    reset_block(RESETS_RESET_USBCTRL_BITS);
    unreset_block_wait(RESETS_RESET_USBCTRL_BITS);

    memset(usb_dpram, 0, sizeof(*usb_dpram));

    irq_set_enabled(USBCTRL_IRQ, false);
    auto old_handler = irq_get_exclusive_handler(USBCTRL_IRQ);
    if (old_handler) {
        irq_remove_handler(USBCTRL_IRQ, old_handler);
    }
    irq_set_exclusive_handler(USBCTRL_IRQ, _irq_handler_usbctrl);
    irq_set_priority(USBCTRL_IRQ, PICO_HIGHEST_IRQ_PRIORITY);
    irq_set_enabled(USBCTRL_IRQ, true);

    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;
    usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;
    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS | USB_INTS_SETUP_REQ_BITS;

    hw_set_alias(usb_hw)->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;

    return true;
}

void rp2040_usb_device::set_setup_packet_callback(bool (*setup_packet_callback)(const struct usb_setup_packet *pkt))
{
    this->setup_packet_callback = setup_packet_callback;
}

void rp2040_usb_device::apply_endpoint_configuration(const struct usb_endpoint_descriptor *ep_desc, void (*transferred_callback)(const void *data, const int len))
{
    const auto ep_addr = ep_desc->bEndpointAddress;
    const auto ep_buf_offset = reinterpret_cast<uintptr_t>(get_usb_ep_buf_ptr(ep_addr)) - reinterpret_cast<uintptr_t>(usb_dpram);
    const uint32_t val = EP_CTRL_ENABLE_BITS | EP_CTRL_INTERRUPT_PER_BUFFER | (ep_desc->bmAttributes << EP_CTRL_BUFFER_TYPE_LSB) | ep_buf_offset;
    this->transferred_callback[get_usb_ep_index(ep_addr)] = transferred_callback;
    ep_next_pid[get_usb_ep_index(ep_addr)] = DATA_PID::DATA0;
    *(get_usb_ep_ctrl_ptr(ep_addr)) = val;
}

bool rp2040_usb_device::is_configured(void)
{
    return configured;
}

void rp2040_usb_device::configure(void)
{
    configured = true;
}

void rp2040_usb_device::ep0_write(const void *data, const int len)
{
    transmit(USB_ENDPOINT_CONTROL_IN, data, len);
}

void rp2040_usb_device::ep_write(const int ep_addr, const void *data, const int len)
{
    transmit(ep_addr, data, len);
}

void rp2040_usb_device::ep_read(const int ep_addr, const int max_len)
{
    receive(ep_addr, max_len);
}

bool rp2040_usb_device::is_ep_buf_full(const int ep_addr)
{
    return *(get_usb_ep_buf_ctrl_ptr(ep_addr)) & USB_BUF_CTRL_FULL;
}

void rp2040_usb_device::ep0_stall(void)
{
    if (is_dir_out(last_setup_packet.bmRequestType)) {
        usb_hw->ep_stall_arm |= 0x02;
        *(get_usb_ep_buf_ctrl_ptr(USB_ENDPOINT_CONTROL_OUT)) |= USB_BUF_CTRL_STALL;
    } else {
        usb_hw->ep_stall_arm |= 0x01;
        *(get_usb_ep_buf_ctrl_ptr(USB_ENDPOINT_CONTROL_IN)) |= USB_BUF_CTRL_STALL;
    }
}
