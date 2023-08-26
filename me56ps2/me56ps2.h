#include "usb_struct.h"

constexpr auto ME56PS2_BCD_USB        = 0x0110U; // USB 1.1
constexpr auto ME56PS2_BCD_DEVICE     = 0x0101U;
constexpr auto ME56PS2_USB_VENDOR_ID  = 0x0590U; // Omron Corp.
constexpr auto ME56PS2_USB_PRODUCT_ID = 0x001aU; // ME56PS2

constexpr auto ME56PS2_COM_EP_ADDR     = 2U;
constexpr auto ME56PS2_COM_EP_ADDR_IN  = ME56PS2_COM_EP_ADDR | USB_DIR_IN;
constexpr auto ME56PS2_COM_EP_ADDR_OUT = ME56PS2_COM_EP_ADDR | USB_DIR_OUT;

constexpr auto MAX_PACKET_SIZE_CONTROL = 64U; // 8 in original ME56PS2
constexpr auto MAX_PACKET_SIZE_BULK    = 64U;

enum ME56PS2_STRING_ID {
    ME56PS2_STRING_ID_MANUFACTURER = 1,
    ME56PS2_STRING_ID_PRODUCT      = 2,
    ME56PS2_STRING_ID_SERIAL       = 3,
};

// W5500-EVB-PICO pinout
enum PINOUT {
    PINOUT_USER_LED       = 25,
    PINOUT_ETHERNET_RESET = 20,
    PINOUT_ETHERNET_SS    = 17,
};

enum class modem_state : int {
    NotInitialized,
    Offline,
    Ringing,
    Calling,
    Online,
    Disconnected,
};

const struct usb_device_descriptor me56ps2_device_descriptor = {
    .bLength            = sizeof(struct usb_device_descriptor),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_DEVICE,
    .bcdUSB             = ME56PS2_BCD_USB,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = MAX_PACKET_SIZE_CONTROL,
    .idVendor           = ME56PS2_USB_VENDOR_ID,
    .idProduct          = ME56PS2_USB_PRODUCT_ID,
    .bcdDevice          = ME56PS2_BCD_DEVICE,
    .iManufacturer      = ME56PS2_STRING_ID_MANUFACTURER,
    .iProduct           = ME56PS2_STRING_ID_PRODUCT,
    .iSerialNumber      = ME56PS2_STRING_ID_SERIAL,
    .bNumConfigurations = 1,
};

struct usb_config_descriptors {
    struct usb_configuration_descriptor config;
    struct usb_interface_descriptor interface;
    struct usb_endpoint_descriptor endpoint_bulk_in;
    struct usb_endpoint_descriptor endpoint_bulk_out;
};

const struct usb_config_descriptors me56ps2_config_descriptors = {
    .config = {
        .bLength             = sizeof(usb_configuration_descriptor),
        .bDescriptorType     = USB_DESCRIPTOR_TYPE_CONFIGURATION,
        .wTotalLength        = sizeof(me56ps2_config_descriptors),
        .bNumInterfaces      = 1,
        .bConfigurationValue = 1,
        .iConfiguration      = 2,
        .bmAttributes        = USB_CONFIG_ATTR_REMOTE_WAKEUP,
        .bMaxPower           = 0x1e, // 60mA
    },
    .interface = {
        .bLength             = sizeof(usb_interface_descriptor),
        .bDescriptorType     = USB_DESCRIPTOR_TYPE_INTERFACE,
        .bInterfaceNumber    = 0,
        .bAlternateSetting   = 0,
        .bNumEndpoints       = 2,
        .bInterfaceClass     = 0xff, // Vendor specific
        .bInterfaceSubClass  = 0xff, // Vendor specific
        .bInterfaceProtocol  = 0xff, // Vendor specific
        .iInterface          = ME56PS2_STRING_ID_PRODUCT,
    },
    .endpoint_bulk_in = {
        .bLength             = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType     = USB_DESCRIPTOR_TYPE_ENDPOINT,
        .bEndpointAddress    = ME56PS2_COM_EP_ADDR_IN,
        .bmAttributes        = USB_ENDPOINT_TRANSFER_TYPE_BULK,
        .wMaxPacketSize      = MAX_PACKET_SIZE_BULK,
        .bInterval           = 0,
    },
    .endpoint_bulk_out = {
        .bLength             = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType     = USB_DESCRIPTOR_TYPE_ENDPOINT,
        .bEndpointAddress    = ME56PS2_COM_EP_ADDR_OUT,
        .bmAttributes        = USB_ENDPOINT_TRANSFER_TYPE_BULK,
        .wMaxPacketSize      = MAX_PACKET_SIZE_BULK,
        .bInterval           = 0,
    }
};

const struct usb_string_descriptor<1> me56ps2_string_descriptor_0 = {
    .bLength = sizeof(me56ps2_string_descriptor_0),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
    .wData = {0x0409},
};

const struct usb_string_descriptor<3> me56ps2_string_descriptor_1 = { // Manufacturer
    .bLength = sizeof(me56ps2_string_descriptor_1),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
    .wData = {u'N', u'/', u'A'},
};

const struct usb_string_descriptor<14> me56ps2_string_descriptor_2 = { // Product
    .bLength = sizeof(me56ps2_string_descriptor_2),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
    .wData = {u'M', u'o', u'd', u'e', u'm', u' ', u'e', u'm', u'u', u'l', u'a', u't', u'o', u'r'},
};

const struct usb_string_descriptor<3> me56ps2_string_descriptor_3 = { // Serial
    .bLength = sizeof(me56ps2_string_descriptor_3),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
    .wData = {u'N', u'/', u'A'},
};

constexpr auto ME56PS2_STRING_DESCRIPTORS_NUM = 4;
const void *me56ps2_string_descriptors[ME56PS2_STRING_DESCRIPTORS_NUM] = {
    &me56ps2_string_descriptor_0,
    &me56ps2_string_descriptor_1,
    &me56ps2_string_descriptor_2,
    &me56ps2_string_descriptor_3,
};
