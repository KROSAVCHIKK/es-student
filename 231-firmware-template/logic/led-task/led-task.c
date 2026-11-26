#include "led-task/led-task.h"

#include "platform/platform.h"
#include "systime/systime.h"

#define LED_DEFAULT_PERIOD_MS 1000

static led_state_t led_state = LED_STATE_BLINK;
static uint32_t led_period_ms = LED_DEFAULT_PERIOD_MS;
static bool led_on = false;
static uint64_t led_last_us = 0;

void led_task_init(void)
{
    led_period_ms = LED_DEFAULT_PERIOD_MS;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (led_state)
    {
    case LED_STATE_OFF:
        led_on = false;
        platform_led_set(false);
        break;
    case LED_STATE_ON:
        led_on = true;
        platform_led_set(true);
        break;
    case LED_STATE_BLINK:
        led_on = true;
        platform_led_set(true);
        led_last_us = systime_us();
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0)
    {
        return false;
    }
    led_period_ms = period_ms;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return led_period_ms;
}

void led_task_next_state(void)
{
    switch (led_state)
    {
    case LED_STATE_OFF:
        led_task_set_state(LED_STATE_ON);
        break;
    case LED_STATE_ON:
        led_task_set_state(LED_STATE_BLINK);
        break;
    case LED_STATE_BLINK:
        led_task_set_state(LED_STATE_OFF);
        break;
    }
}

void led_task_handle(void)
{
    switch (led_state)
    {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
    {
        uint64_t half_period_us = (uint64_t)led_period_ms * 500ULL;
        if (systime_period_elapsed(&led_last_us, half_period_us))
        {
            led_on = !led_on;
            platform_led_set(led_on);
        }
        break;
    }
    }
}
