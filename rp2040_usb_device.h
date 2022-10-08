#include "usb_struct.h"

enum class DATA_PID : uint8_t {
    DATA0 = 0,
    DATA1 = 1,
};

class rp2040_usb_device
{
    private:
        static rp2040_usb_device *instance;
        static int _dummy_printf(const char *fmt, ...);
        static void _irq_handler_usbctrl(void);
        static void _ep0_in_transferred_callback(const void *data, const int len);

        bool is_dir_out(uint8_t value) {return (value & USB_DIR_BIT_MASK) == USB_DIR_OUT;}
        bool is_dir_in(uint8_t value) {return !is_dir_out(value);}
        bool is_dir_out_by_index(uint8_t index) {return !is_dir_in_by_index(index);}
        bool is_dir_in_by_index(uint8_t index) {return (index & 0x01) == 0;}
        bool is_control(uint8_t ep_addr) {return get_usb_ep_num(ep_addr) == USB_ENDPOINT_CONTROL;}
        int get_usb_ep_index(uint8_t ep_addr) {return ((ep_addr & 0x0f) << 1) | (is_dir_out(ep_addr) ? 0x01 : 0x00);}
        uint8_t get_usb_ep_num(uint8_t ep_addr) {return ep_addr & USB_ENDPOINT_NUMBER_BIT_MASK;}
        uint8_t get_usb_ep_num_by_index(uint8_t index) {return index >> 1;}
        uint8_t get_usb_ep_addr_by_index(uint8_t index) {return get_usb_ep_num_by_index(index) | (is_dir_out_by_index(index) ? USB_DIR_OUT : USB_DIR_IN);}
        io_rw_32 *get_usb_ep_buf_ctrl_ptr(uint8_t ep_addr)
        {
            return is_dir_out(ep_addr) ? &usb_dpram->ep_buf_ctrl[get_usb_ep_num(ep_addr)].out : &usb_dpram->ep_buf_ctrl[get_usb_ep_num(ep_addr)].in;
            //auto *ep_buf_ctrl = &usb_dpram->ep_buf_ctrl[get_usb_ep_num(ep_addr)];
            //return is_dir_out(ep_addr) ? &ep_buf_ctrl->out : &ep_buf_ctrl->in;
        }
        io_rw_32 *get_usb_ep_ctrl_ptr(uint8_t ep_addr)
        {
            auto *ep_ctrl = &usb_dpram->ep_ctrl[get_usb_ep_num(ep_addr) - 1];
            return is_dir_out(ep_addr) ? &ep_ctrl->out : &ep_ctrl->in;
        }
        void *get_usb_ep_buf_ptr(uint8_t ep_addr)
        {
            return is_control(ep_addr)
                ? reinterpret_cast<void *>(usb_dpram->ep0_buf_a)
                : reinterpret_cast<void *>(&usb_dpram->epx_data[(get_usb_ep_index(ep_addr) - 2) * 64]);
        }
        void clear_sie_status(uint32_t clear_bit) {hw_clear_alias(usb_hw)->sie_status = clear_bit;}

        DATA_PID ep_next_pid[32];

        struct usb_setup_packet last_setup_packet;
        bool configured;

        bool (*setup_packet_callback)(const struct usb_setup_packet *pkt);
        void (*transferred_callback[32])(const void *data, const int len);

        void dump_hex_and_ascii(const void *data, const size_t length);
        void bus_reset(void);
        void ep0_in_transferred_callback(const void *data, const int len);
        void irq_handler_usbctrl(void);
        void transmit(const uint8_t ep_addr, const void *data, const int len);
        void receive(const uint8_t ep_addr, const int max_len);
        void handle_setup_packet(const volatile struct usb_setup_packet *pkt);
        void handle_buff_status();
        uint32_t get_ep_pid(const uint8_t ep_addr);
        int (*printf)(const char *fmt, ...);
    public:
        rp2040_usb_device(int (*printf)(const char *fmt, ...) = nullptr);
        bool init(void);
        void set_setup_packet_callback(bool (*setup_packet_callback)(const struct usb_setup_packet *pkt));
        void apply_endpoint_configuration(const struct usb_endpoint_descriptor *ep_desc, void (*transferred_callback)(const void *data, const int len));
        bool is_configured(void);
        void configure(void);
        void ep0_write(const void *data, const int len);
        void ep_write(const int ep_num, const void *data, const int len);
        void ep_read(const int ep_num, const int max_len);
        bool is_ep_buf_full(const int ep_addr);
        void ep0_stall(void);
};
