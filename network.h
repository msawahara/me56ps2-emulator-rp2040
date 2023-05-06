#pragma once

void initialize_network(void);

class client_socket;

class server_socket
{
    public:
        virtual client_socket *accept(void) = 0;
};

class client_socket
{
    public:
        virtual ~client_socket() = 0;
        virtual size_t write(const char *buf, const size_t len) = 0;
        virtual size_t read(char *buf, const size_t max_len) = 0;
        virtual size_t available_write(void) = 0;
        virtual size_t available_read(void) = 0;
        virtual void close(void) = 0;
};

class tcp_server_socket: public server_socket
{
    protected:
        tcp_server_socket();
    public:
        tcp_server_socket(const uint16_t port);
};

class tcp_client_socket: public client_socket
{
    public:
        virtual ~tcp_client_socket() = 0;
        virtual size_t write(const char *buf, const size_t len) = 0;
        virtual size_t read(char *buf, const size_t max_len) = 0;
        virtual size_t available_write(void) = 0;
        virtual size_t available_read(void) = 0;
        virtual void close(void) = 0;
};

server_socket *create_tcp_server_socket(const uint16_t port);
