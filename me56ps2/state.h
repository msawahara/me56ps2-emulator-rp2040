#pragma once

#include "lock.h"

template <typename T>
class state_ctrl {
    private:
        critical_section_t cs;
        T state;
    public:
        state_ctrl(const T initial);
        ~state_ctrl();
        T get_state();
        bool is_state (const T state);
        bool transition(const T current_state, const T next_state);
        void force_transition(const T next_state);
};

template <typename T>
state_ctrl<T>::state_ctrl(const T initial)
{
    critical_section_init(&cs);
    state = initial;
}

template <typename T>
state_ctrl<T>::~state_ctrl()
{
    critical_section_deinit(&cs);
}

template <typename T>
T state_ctrl<T>::get_state()
{
    lock_guard lk(&cs);

    return state;
}

template <typename T>
bool state_ctrl<T>::is_state(const T state)
{
    lock_guard lk(&cs);

    return this->state == state;
}

template <typename T>
bool state_ctrl<T>::transition(const T current_state, const T next_state)
{
    lock_guard lk(&cs);

    if (state == current_state) {
        state = next_state;
        return true;
    }

    return false;
}

template <typename T>
void state_ctrl<T>::force_transition(const T next_state)
{
    lock_guard lk(&cs);

    state = next_state;
}
