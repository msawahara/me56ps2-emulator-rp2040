#pragma once

#include "pico/critical_section.h"

class lock_guard
{
    private:
        critical_section_t *cs;
    public:
        lock_guard(critical_section_t *cs);
        ~lock_guard();
};

inline lock_guard::lock_guard(critical_section_t *cs)
{
    this->cs = cs;
    critical_section_enter_blocking(cs);
}

inline lock_guard::~lock_guard()
{
    critical_section_exit(cs);
}
