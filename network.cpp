#include <stdint.h>

// for WIZnet W5500-EVB-PICO
#if defined(ARDUINO_WIZNET_5500_EVB_PICO)

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <utility/w5100.h>

#include "network_w5500.h"

// for Raspberry Pi Pico W
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)

#include <WiFi.h>

#include "network_pico_w.h"

// for other devices.
#else

#error "Unsupported device."

#endif

#include "pico/unique_id.h"
#include "config.h"
#include "me56ps2.h"

void generate_board_mac_address(uint8_t *mac)
{
    pico_unique_board_id_t uid;
    pico_get_unique_board_id(&uid);

    mac[3] = uid.id[0] ^ uid.id[3] ^ uid.id[6];
    mac[4] = uid.id[1] ^ uid.id[4] ^ uid.id[7];
    mac[5] = uid.id[2] ^ uid.id[5];
}

// for WIZnet W5500-EVB-PICO

#if defined(ARDUINO_WIZNET_5500_EVB_PICO)

void set_ethernet_reset(bool value)
{
    pinMode(PINOUT_ETHERNET_RESET, OUTPUT);
    digitalWrite(PINOUT_ETHERNET_RESET, !value);
}

void w5x00_write_uint8(const uint16_t addr, uint8_t value)
{
    W5100.write(addr, value);
}

void w5x00_write_uint16(const uint16_t addr, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = value >> 8;
    buf[1] = value & 0xff;
    W5100.write(addr, buf, 2);
}

void set_retry_register(const uint16_t retry_time_value, const uint8_t retry_count)
{
    switch (Ethernet.hardwareStatus()) {
        case EthernetW5100:
            w5x00_write_uint16(0x0017, retry_time_value);
            w5x00_write_uint8(0x0019, retry_count);
            break;
        case EthernetW5500:
            w5x00_write_uint16(0x0019, retry_time_value);
            w5x00_write_uint8(0x001b, retry_count);
            break;
    }
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

    set_retry_register(retry_time_value, retry_count);
}


#endif

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)

void initialize_network(void)
{  
    using namespace config;
  
    if (use_dhcp) {
      WiFi.config(ip_addr, dns_server, gateway, subnet_mask);
    }
  
    WiFi.begin(wifi::ssid, wifi::passphrase);
}

#endif

server_socket *create_tcp_server_socket(const uint16_t port)
{
#if defined(ARDUINO_WIZNET_5500_EVB_PICO)
    return new tcp_server_socket_w5500(port);
#endif
}





