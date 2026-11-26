#include "systime/systime.h"

#include "pico/stdlib.h"

uint64_t systime_us(void)
{
    return time_us_64();
}

uint64_t systime_ms(void)
{
    return time_us_64() / 1000ULL;
}

bool systime_period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now = systime_us();
    uint64_t elapsed = now - *last_us;
    if (elapsed < period_us)
    {
        return false;
    }

    if (elapsed >= 2ULL * period_us)
    {
        *last_us = now;
    }
    else
    {
        *last_us += period_us;
    }
    return true;
}
