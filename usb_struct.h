#pragma once

enum USB_DIR {
    USB_DIR_BIT_MASK = 0x80,
    USB_DIR_OUT      = 0x00,
    USB_DIR_IN       = 0x80,
};

enum USB_ENDPOINT {
    USB_ENDPOINT_NUMBER_BIT_MASK = 0x0f,
    USB_ENDPOINT_CONTROL         = 0x00,
    USB_ENDPOINT_CONTROL_OUT     = USB_ENDPOINT_CONTROL | USB_DIR_OUT,
    USB_ENDPOINT_CONTROL_IN      = USB_ENDPOINT_CONTROL | USB_DIR_IN,
};

enum USB_REQUEST_TYPE {
    USB_REQUEST_TYPE_BIT_MASK = 0x03 << 5,
    USB_REQUEST_TYPE_STANDARD = 0x00 << 5,
    USB_REQUEST_TYPE_CLASS    = 0x01 << 5,
    USB_REQUEST_TYPE_VENDOR   = 0x02 << 5,
    USB_REQUEST_TYPE_RESERVED = 0x03 << 5,
};

enum USB_REQUEST {
    USB_REQUEST_SET_ADDRESS       = 5,
    USB_REQUEST_GET_DESCRIPTOR    = 6,
    USB_REQUEST_GET_CONFIGURATION = 8,
    USB_REQUEST_SET_CONFIGURATION = 9,
    USB_REQUEST_SET_INTERFACE     = 11,
};

enum USB_DESCRIPTOR_TYPE {
    USB_DESCRIPTOR_TYPE_DEVICE        = 1,
    USB_DESCRIPTOR_TYPE_CONFIGURATION = 2,
    USB_DESCRIPTOR_TYPE_STRING        = 3,
    USB_DESCRIPTOR_TYPE_INTERFACE     = 4,
    USB_DESCRIPTOR_TYPE_ENDPOINT      = 5,
};

enum USB_CONFIG_ATTR {
    USB_CONFIG_ATTR_FIXED_BIT     = 0x80,
    USB_CONFIG_ATTR_SELF_POWERED  = 0x40,
    USB_CONFIG_ATTR_REMOTE_WAKEUP = 0x20,
};

enum USB_ENDPOINT_TRANSFER_TYPE {
    USB_ENDPOINT_TRANSFER_TYPE_BIT_MASK    = 0x03,
    USB_ENDPOINT_TRANSFER_TYPE_CONTROL     = 0x00,
    USB_ENDPOINT_TRANSFER_TYPE_ISOCHRONOUS = 0x01,
    USB_ENDPOINT_TRANSFER_TYPE_BULK        = 0x02,
    USB_ENDPOINT_TRANSFER_TYPE_INTERRUPT   = 0x03,
};

struct usb_setup_packet {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __packed;

struct usb_device_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __packed;

struct usb_configuration_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __packed;

struct usb_interface_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __packed;

struct usb_endpoint_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __packed;

template<int N>
struct usb_string_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    char16_t wData[N];
} __packed;
