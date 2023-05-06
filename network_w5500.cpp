
#include <cstddef>
#include "network_w5500.h"

tcp_server_socket_w5500::tcp_server_socket_w5500(const uint16_t port)
{
    server = new EthernetServer(port);
}

tcp_server_socket_w5500::~tcp_server_socket_w5500()
{
    delete server;
}

client_socket *tcp_server_socket_w5500::accept(void)
{
    EthernetClient client = server->accept();
    if (!client) {
        return nullptr;
    }

    return new tcp_client_socket_w5500(client);
}

tcp_client_socket_w5500::tcp_client_socket_w5500(const EthernetClient &client)
{
    this->client = new EthernetClient(client);
}

tcp_client_socket_w5500::~tcp_client_socket_w5500()
{
    delete client;
}

size_t tcp_client_socket_w5500::write(const char *buf, const size_t len)
{
    return client->write(reinterpret_cast<const uint8_t *>(buf), len);
}

size_t tcp_client_socket_w5500::read(char *buf, const size_t max_len)
{
    return client->read(reinterpret_cast<uint8_t *>(buf), max_len);
}

size_t tcp_client_socket_w5500::available_write(void)
{
    return client->availableForWrite();
}

size_t tcp_client_socket_w5500::available_read(void)
{
    return client->available();
}

void tcp_client_socket_w5500::close(void)
{
    client->stop();
}