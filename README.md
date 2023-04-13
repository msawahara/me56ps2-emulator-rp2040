# me56ps2-emulator-rp2040
[日本語版はこちらを開いてください](/README_ja.md)

This software is a port of [me56ps2-emulator](https://github.com/msawahara/me56ps2-emulator) for use with WIZnet W5500-EVB-PICO.

## Setup instructions
See [SETUP.md](SETUP.md).

## Setting
See [CONFIGURATION.md](CONFIGURATION.md).

## Specify connection destination
You can specify an IP address instead of a phone number when connecting a modem. <br>
Use hyphens ( `-` ) as delimiters in IP addresses. <br>
When specifying a port number, follow the IP address with a pound (`#`) and the port number.

Example: Destination IP = 198.51.100.234, Port number = 8080
```
198-51-100-234#8080
```
