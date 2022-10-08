#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>

#include <algorithm>
#include <cstdarg>

#include "hardware/regs/usb.h"
#include "hardware/structs/usb.h"
#include "hardware/irq.h"
#include "hardware/resets.h"
#include "pico/unique_id.h"

#include "usb_struct.h"
#include "ring_buffer.h"
#include "state.h"
#include "rp2040_usb_device.h"
#include "me56ps2.h"
#include "config.h"

ring_buffer<char> usb_rx_buffer(8192);
ring_buffer<char> usb_tx_buffer(8192);
ring_buffer<char> net_rx_buffer(8192);
ring_buffer<char> net_tx_buffer(8192);
ring_buffer<char> log_tx_buffer(2048); // for debugging

rp2040_usb_device *usb;
IPAddress server_ip;
uint16_t server_port;
EthernetServer server(config::listen_port);
EthernetClient client;
EthernetServer log_server(config::log_listen_port);
EthernetClient log_client;
state_ctrl<modem_state> state(modem_state::NotInitialized);

int _printf(const char *fmt, ...)
{
    va_list args;
    char buf[256];

    if (!config::enable_log) {return 0;}

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    log_tx_buffer.enqueue(buf, strlen(buf));
    return Serial1.print(buf);
}

void ep2_out_handler(const void *data, const int len)
{
    int payload_length = len - 1;
    usb_rx_buffer.enqueue(reinterpret_cast<const char *>(data) + 1, payload_length);

    usb->ep_read(ME56PS2_COM_EP_ADDR_OUT, MAX_PACKET_SIZE_BULK);
}

bool control_packet_handler(const struct usb_setup_packet* pkt)
{
    const auto req_type = static_cast<USB_REQUEST_TYPE>(pkt->bmRequestType & USB_REQUEST_TYPE_BIT_MASK);
    const auto req = static_cast<USB_REQUEST>(pkt->bRequest);

    if (req_type == USB_REQUEST_TYPE_STANDARD) {
        if (req == USB_REQUEST_GET_DESCRIPTOR) {
            const auto desc_type = static_cast<USB_DESCRIPTOR_TYPE>(pkt->wValue >> 8);
            const auto desc_idx = pkt->wValue & 0x00ff;
            if (desc_type == USB_DESCRIPTOR_TYPE_DEVICE) {
                const auto len = std::min(me56ps2_device_descriptor.bLength, static_cast<uint8_t>(pkt->wLength));
                usb->ep0_write(&me56ps2_device_descriptor, len);
                return true;
            }
            if (desc_type == USB_DESCRIPTOR_TYPE_CONFIGURATION) {
                const auto len = std::min(me56ps2_config_descriptors.config.wTotalLength, static_cast<uint16_t>(pkt->wLength));
                usb->ep0_write(&me56ps2_config_descriptors, len);
                return true;
            }
            if (desc_type == USB_DESCRIPTOR_TYPE_STRING) {
                if (desc_idx >= ME56PS2_STRING_DESCRIPTORS_NUM) {return false;} // invalid string id
                const auto desc_len = reinterpret_cast<const struct usb_string_descriptor<1> *>(me56ps2_string_descriptors[desc_idx])->bLength;
                const auto len = std::min(desc_len, static_cast<typeof(desc_len)>(pkt->wLength));
                usb->ep0_write(me56ps2_string_descriptors[desc_idx], len);
                return true;
            }
        }
        if (req == USB_REQUEST_SET_CONFIGURATION) {
            usb->apply_endpoint_configuration(&me56ps2_config_descriptors.endpoint_bulk_in, nullptr);
            usb->apply_endpoint_configuration(&me56ps2_config_descriptors.endpoint_bulk_out, ep2_out_handler);
            usb->configure();
            usb->ep0_write(nullptr, 0);
            usb->ep_read(ME56PS2_COM_EP_ADDR_OUT, MAX_PACKET_SIZE_BULK);
            state.force_transition(modem_state::Offline);
            return true;
        }
        if (req == USB_REQUEST_SET_INTERFACE) {
            usb->ep0_write(nullptr, 0);
            return true;
        }
    }
    if (req_type == USB_REQUEST_TYPE_VENDOR) {
        if (pkt->bRequest == 0x01) {
            if ((pkt->wValue & 0x0101) == 0x0100) {
                // set DTR to LOW for on-hook
                _printf("on-hook\r\n");
                usb_tx_buffer.clear();
                usb_rx_buffer.clear();
                state.force_transition(modem_state::Offline);
            } else if ((pkt->wValue & 0x0101) == 0x0101) {
                // set DTR to HIGH for off-hook
                _printf("off-hook\r\n");
            }
        }
        usb->ep0_write(nullptr, 0);
        return true;
    }

    return false;
}

bool parse_address(const char *addr, IPAddress *ip_addr, uint16_t *oport)
{
    // Input format: "000-000-000-000#00000" or "000-000-000-000"
    int d[4] = {0, 0, 0, 0};
    int port = config::default_port;

    // Parse IPv4 address
    auto ret = sscanf(addr, "%u-%u-%u-%u#%u", &d[0], &d[1], &d[2], &d[3], &port);
    if (ret < 4) {return false;}

    // Check each digit range
    for (int i = 0; i < 4; i++) {
        if (d[i] < 0 || d[i] > 255) {return false;}
    }

    // Check port range (1 - 65535)
    if (port < 1 || port > 65535) {return false;}

    *ip_addr = IPAddress(d[0], d[1], d[2], d[3]);
    *oport = port;

    return true;
}

void usb_rx_process()
{
    if (state.is_state(modem_state::Online)) {
        net_tx_buffer.pull(&usb_rx_buffer);
        return;
    }

    if (state.is_state(modem_state::Calling)) {
        return;
    }

    char line[64];
    size_t len = 0;
    while (usb_rx_buffer.find('\x0d', &len)) {
        if (len > sizeof(line) - 1) {
            // Ignore too long line
            usb_rx_buffer.erase(len);
            continue;
        }

        usb_rx_buffer.dequeue(line, len);
        line[len] = 0;

        if (strncmp(line, "ATD", 3) == 0) {
            // Dial
            if (len > 4 && parse_address(&line[4], &server_ip, &server_port)) {
                state.transition(modem_state::Offline, modem_state::Calling);
                break;
            } else {
                const char reply[] = "BUSY\r\n";
                usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
            }
        } else if (strncmp(line, "ATA", 3) == 0) {
            // Answer an incoming call
            if (state.transition(modem_state::Ringing, modem_state::Online)) {
                const char reply[] = "CONNECT 33600 V42\r\n";
                usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
                break;
            } else {
                const char reply[] = "ERROR\r\n";
                usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
            }
        } else {
            const char reply[] = "OK\r\n";
            usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
        }
    }
}

void usb_tx_process()
{
    static unsigned long last_sent_time = 0;

    if (usb->is_ep_buf_full(ME56PS2_COM_EP_ADDR_IN)) {return;}
    if (usb_tx_buffer.is_empty() && millis() - config::report_interval_ms < last_sent_time) {return;}
    last_sent_time = millis();

    char tx_packet[MAX_PACKET_SIZE_BULK] = {0x31, 0x60};
    if (state.is_state(modem_state::Online)) {tx_packet[0] |= 0x80;}
    int tx_packet_len = 2 + usb_tx_buffer.dequeue(&tx_packet[2], MAX_PACKET_SIZE_BULK - 2);
    usb->ep_write(ME56PS2_COM_EP_ADDR_IN, tx_packet, tx_packet_len);
}

void setup()
{
    Serial1.begin();
    Serial1.printf("Boot.\r\n");
    Serial1.printf("Initializing USB Device...\r\n");
    usb = new rp2040_usb_device(_printf);
    usb->set_setup_packet_callback(control_packet_handler);
    usb->init();
}

void loop()
{
    delay(1);

    if (!usb->is_configured()) {return;}

    usb_rx_process();

    if (state.is_state(modem_state::Online)) {
        usb_tx_buffer.pull(&net_rx_buffer);
    }

    usb_tx_process();
}

void set_user_led(bool value)
{
    pinMode(PINOUT_USER_LED, OUTPUT);
    digitalWrite(PINOUT_USER_LED, value);
}

void set_ethernet_reset(bool value)
{
    pinMode(PINOUT_ETHERNET_RESET, OUTPUT);
    digitalWrite(PINOUT_ETHERNET_RESET, !value);
}

void generate_board_mac_address(uint8_t *mac)
{
    pico_unique_board_id_t uid;
    pico_get_unique_board_id(&uid);

    mac[3] = uid.id[0] ^ uid.id[3] ^ uid.id[6];
    mac[4] = uid.id[1] ^ uid.id[4] ^ uid.id[7];
    mac[5] = uid.id[2] ^ uid.id[5];
}

void initialize_network(void)
{
    using namespace config;

    if (use_board_unique_id) {
        generate_board_mac_address(mac_addr);
    }

    set_ethernet_reset(true);
    delay(10);
    set_ethernet_reset(false);
    delay(10);
    Ethernet.init(PINOUT_ETHERNET_SS);

    if (use_dhcp) {
        Ethernet.begin(mac_addr);
    } else {
        using namespace static_ip;
        Ethernet.begin(mac_addr, ip_addr, dns_server, gateway, subnet_mask);
    }
}

void setup1()
{
    set_user_led(false);

    initialize_network();

    const auto *mac = config::mac_addr;
    Serial1.printf("Ethernet Hardware Status: %d\r\n", Ethernet.hardwareStatus());
    Serial1.printf("IP Address: %s\r\n", Ethernet.localIP().toString().c_str());
    Serial1.printf("MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    set_user_led(true);

    log_server.begin();
}

void log_tx()
{
    EthernetClient new_client = log_server.accept();
    if (new_client) {
        if (!log_client.connected()) {
            log_client.stop();
            log_client = new_client;
            log_tx_buffer.clear();
        } else {
            new_client.stop();
        }
    }

    while (log_client.availableForWrite() && !log_tx_buffer.is_empty()) {
        char buf[64];
        int len = log_tx_buffer.dequeue(buf, sizeof(buf));
        int ptr = 0;
        while (ptr < len) {
            ptr += log_client.write(buf + ptr, len - ptr);
        }
    }
}

void loop1()
{
    if (config::enable_log) {log_tx();}

    EthernetClient new_client = server.accept();
    if (new_client) {
        if (state.transition(modem_state::Offline, modem_state::Ringing)) {
            client = new_client;
            const char msg[] = "RING\r\n";
            usb_tx_buffer.enqueue(msg, sizeof(msg) - 1);
        } else {
            new_client.stop();
        }
    }

    if (state.is_state(modem_state::Calling)) {
        _printf("Connecting...\r\n");
    
        if (client.connect(server_ip, server_port)) {
            _printf("Connected.\r\n");
            const char reply[] = "CONNECT 33600 V.42\r\n";
            usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
            state.transition(modem_state::Calling, modem_state::Online);
        } else {
            _printf("Connection failed.\r\n");
            const char reply[] = "BUSY\r\n";
            usb_tx_buffer.enqueue(reply, sizeof(reply) - 1);
            state.transition(modem_state::Calling, modem_state::Offline);
        }
    }

    if (state.is_state(modem_state::Offline)) {
        if (client.availableForWrite()) {
            client.stop();
        }
    }

    if (!state.is_state(modem_state::Online)) {
        return;
    }

    // Receive
    while (client.available() && !net_rx_buffer.is_full()) {
        char buf[64];
        const auto max_len = std::min(sizeof(buf), net_rx_buffer.get_free_count());
        const auto len = client.read(reinterpret_cast<uint8_t *>(buf), max_len);
        net_rx_buffer.enqueue(buf, len);
    }

    // Transmit
    while (client.availableForWrite() && !net_tx_buffer.is_empty()) {
        char buf[512];
        const auto max_len = std::min(sizeof(buf), static_cast<size_t>(client.availableForWrite()));
        int len = net_tx_buffer.dequeue(buf, max_len);
        int ptr = 0;
        while (ptr < len) {
            ptr += client.write(buf + ptr, len - ptr);
        }
    }

    if (!client.connected()) {
        state.transition(modem_state::Online, modem_state::Disconnected);
    }
}
