// for WIZnet W5500-EVB-PICO
#pragma once

#include <cstdint>

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <utility/w5100.h>

#include "network.h"

class tcp_client_socket_w5500;

class tcp_server_socket_w5500: public tcp_server_socket
{
    EthernetServer *server;
    public:
        tcp_server_socket_w5500(const uint16_t port);
        ~tcp_server_socket_w5500();
        client_socket *accept(void);
};

class tcp_client_socket_w5500: public tcp_client_socket
{
    private:
        EthernetClient *client;
    public:
        tcp_client_socket_w5500(const EthernetClient &client);
        ~tcp_client_socket_w5500();
        size_t write(const char *buf, const size_t len);
        size_t read(char *buf, const size_t max_len);
        size_t available_write(void);
        size_t available_read(void);
        void close(void);
};