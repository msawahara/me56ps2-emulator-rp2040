#pragma once

#include <cstddef>

#include "lock.h"

template <typename T>
class ring_buffer
{
    private:
        size_t write_ptr, read_ptr;
        size_t buffer_size;
        T *buffer;
        critical_section_t cs;
        bool is_empty_without_lock(void);
        bool is_full_without_lock(void);
        bool enqueue_signle_without_lock(const T *data);
        bool dequeue_signle_without_lock(T *data);
    public:
        ring_buffer(const size_t size);
        ~ring_buffer();
        bool is_empty(void);
        bool is_full(void);
        size_t get_buffer_size(void);
        size_t get_count(void);
        size_t get_free_count(void);
        size_t enqueue(const T *data, size_t length);
        size_t dequeue(T *data, size_t max_length);
        size_t erase(size_t length);
        size_t pull(ring_buffer<T> *from);
        void clear(void);
        bool find(const T marker, size_t *length);
};

template <typename T>
ring_buffer<T>::ring_buffer(const size_t size)
{
    write_ptr = 0;
    read_ptr = 0;
    buffer_size = size;
    buffer = new T[size];
    critical_section_init(&cs);
}

template <typename T>
ring_buffer<T>::~ring_buffer()
{
    critical_section_deinit(&cs);
    delete buffer;
}

template <typename T>
bool ring_buffer<T>::is_empty_without_lock(void)
{
    return write_ptr == read_ptr;
}

template <typename T>
bool ring_buffer<T>::is_full_without_lock(void)
{
    const auto next_write_ptr = (write_ptr + 1) % buffer_size;
    return next_write_ptr == read_ptr;
}

template <typename T>
bool ring_buffer<T>::is_empty(void)
{
    lock_guard lk(&cs);

    return is_empty_without_lock();
}

template <typename T>
bool ring_buffer<T>::is_full(void)
{
    lock_guard lk(&cs);

    return is_full_without_lock();
}

template <typename T>
size_t ring_buffer<T>::get_buffer_size(void)
{
    return buffer_size - 1;
}

template <typename T>
size_t ring_buffer<T>::get_count(void)
{
    lock_guard lk(&cs);

    return (buffer_size + write_ptr - read_ptr) % buffer_size;
}

template <typename T>
size_t ring_buffer<T>::get_free_count(void)
{
    return get_buffer_size() - get_count();
}

template <typename T>
bool ring_buffer<T>::enqueue_signle_without_lock(const T *data)
{
    if (is_full_without_lock()) {
        return false;
    }

    buffer[write_ptr] = *data;
    write_ptr = (write_ptr + 1) % buffer_size;

    return true;
}

template <typename T>
size_t ring_buffer<T>::enqueue(const T *data, size_t length)
{
    lock_guard lk(&cs);

    size_t ptr = 0;
    while(ptr < length && enqueue_signle_without_lock(&data[ptr])) {ptr++;}

    return ptr;
}

template <typename T>
bool ring_buffer<T>::dequeue_signle_without_lock(T *data)
{
    if (is_empty_without_lock()) {
        return false;
    }

    *data = buffer[read_ptr];
    read_ptr = (read_ptr + 1) % buffer_size;

    return true;
}

template <typename T>
size_t ring_buffer<T>::dequeue(T *data, size_t max_length)
{
    lock_guard lk(&cs);

    size_t ptr = 0;
    while (ptr < max_length && dequeue_signle_without_lock(&data[ptr])) {ptr++;}

    return ptr;
}

template <typename T>
size_t ring_buffer<T>::erase(size_t length)
{
    lock_guard lk(&cs);

    const auto erased = std::min(length, get_count());
    read_ptr += erased;

    return erased;
}

template <typename T>
size_t ring_buffer<T>::pull(ring_buffer<T> *from)
{
    lock_guard lk(&cs);
    lock_guard lk2(&from->cs);
    size_t count = 0;
    T data;

    while (!is_full_without_lock() && from->dequeue_signle_without_lock(&data)) {
        enqueue_signle_without_lock(&data);
        count++;
    }

    return count;
}

template <typename T>
void ring_buffer<T>::clear(void)
{
    lock_guard lk(&cs);

    read_ptr = write_ptr;
}

template <typename T>
bool ring_buffer<T>::find(const T marker, size_t *length)
{
    lock_guard lk(&cs);

    for (auto ptr = read_ptr; ptr != write_ptr; ptr = (ptr + 1) % buffer_size) {
        if (buffer[ptr] == marker) {
            *length = (buffer_size + ptr - read_ptr + 1) % buffer_size;
            return true;
        }
    }
    return false;
}
