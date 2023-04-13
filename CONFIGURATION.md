# Configuration
In many cases, it can be used without changing the settings because it works with automatic settings by DHCP. <br>
However, if you want to use a static IP address or change the port number to use it as a server, you can change the settings. <br>

Settings are written in the `config.h` file.

## DHCP
```c++
     // When setting the address with DHCP: true
     // If setting a static IP address: false
     constexpr bool use_dhcp = true;
```
Specify `true` for `use_dhcp` to enable DHCP. <br>
To disable DHCP and use a static IP address, set `use_dhcp` to `false`.

## static IP address
```c++
     // static IP address
     namespace static_ip {
         const IPAddress ip_addr(192, 168, 1, 2);
         const IPAddress dns_server(192, 168, 1, 1);
         const IPAddress gateway(192, 168, 1, 1);
         const IPAddress subnet_mask(255, 255, 255, 0);
     }
```
When specifying a static IP address, the following four items must be set.
- `ip_addr` : IP address used by the device
- `dns_server` : DNS server IP address
- `gateway` : IP address of the gateway
- `subnet_mask` : Subnet mask (IP address format)

Only IPv4 addresses can be used for the IP address. <br>
When putting them in `config.h`, put them in parentheses and separate them with commas.

## port number to use for the match
```c++
     // Match listening port
     constexpr uint16_t listen_port = 10023;

     // Default port for match connection
     constexpr uint16_t default_port = 10023;
```
`listen_port` is the port number to listen on. When using it as a server, change it as necessary. <br>
`default_port` is the destination port number to be used if the port number is omitted when specifying the destination.

## debug log output
```c++
     // enable logging
     constexpr bool enable_log = false;

     // log output listening port (for debugging)
     constexpr uint16_t log_listen_port = 23;
```
The debug log output setting is disabled by default. <br>
If enabled, UART transmission wait will occur during log output, which may affect the operation of the game.

Set `enable_log` to `true` to enable logging.
Then, the log is output in the following two ways.
- UART (TX = 1 pin, RX = 2 pin, 115200bps, 8bits, no parity, 1 stop bit)
- TCP port 23 (port number specified in `log_listen_port`)

## MAC address
```c++
     // MAC address
     uint8_t mac_addr[6] = {0x02, 0x20, 0x40, 0x00, 0x00, 0x00};

     // Automatically generate the lower 3-octets of the MAC address based on the Board Unique ID
     constexpr bool use_board_unique_id = true;
```
The MAC address uses a local MAC address starting with `02-20-40`. <br>
In addition, the lower 3-octets address is generated based on the Board Unique ID so that MAC addresses do not overlap when using multiple devices.

## Transmission interval during inactivity
```c++
     // Transmission interval when inactive
     constexpr int report_interval_ms = 40;
```
This is the interval for reporting the modem status information when there is no data to be sent via USB transmission. <br>
Normally you do not need to change.

___Note: This setting is different from the latency timer. ___

In the original ME56PS2, when the data in the USB transmission buffer is less than 1 packet (62 bytes; 64 bytes minus 2 bytes for the header), the packet transmission waits until the latency timer times out. was designed to The initial value of this latency timer is 40ms, which causes transmission delays.

The me56ps2-emulator-rp2040 implementation immediately sends the data in the buffer even if the data in the send buffer is less than one packet. This operation reduces the transfer efficiency of the USB bus, but enables transmission with lower latency.

In other words, me56ps2-emulator-rp2040 does not have a latency timer setting. <br>
Unlike the latency timer, `report_interval_ms` does not delay data transmission, but sets the transmission interval when there is no data to be transmitted.
