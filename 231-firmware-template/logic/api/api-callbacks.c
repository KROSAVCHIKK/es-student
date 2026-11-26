#include "api/api-commands.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "firmware.h"
#include "device/device.h"
#include "systime/systime.h"
#include "profiling/profiling.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"

#define PI_DEFAULT_TERMS 1000000U

static bool parse_u32(const char *str, uint32_t *out)
{
    if (str == NULL || *str == '\0')
    {
        return false;
    }

    for (const char *p = str; *p != '\0'; p++)
    {
        if (*p < '0' || *p > '9')
        {
            return false;
        }
    }

    errno = 0;
    char *end = NULL;
    unsigned long val = strtoul(str, &end, 10);
    if (errno == ERANGE || end == str || *end != '\0' || val > UINT32_MAX)
    {
        return false;
    }

    *out = (uint32_t)val;
    return true;
}

static const char *led_state_name(led_state_t state)
{
    switch (state)
    {
    case LED_STATE_OFF:
        return "off";
    case LED_STATE_ON:
        return "on";
    case LED_STATE_BLINK:
        return "blink";
    }
    return "unknown";
}

static void print_led_status(void)
{
    printf("led: %s, period %u ms\r\n",
           led_state_name(led_task_get_state()),
           (unsigned)led_task_get_period_ms());
}

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info_t info;
    device_get_info(&info);

    printf("name: %s\r\n", FIRMWARE_NAME);
    printf("version: %s\r\n", FIRMWARE_VERSION);
    printf("project: %s\r\n", FIRMWARE_PROJECT);
    printf("repo: %s\r\n", FIRMWARE_REPO);
    printf("board: %s\r\n", info.board);
    printf("serial: %s\r\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\r\n",
           (unsigned)info.manufacturer,
           (unsigned)info.part,
           (unsigned)info.revision);
    printf("pico-sdk: %s\r\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\r\n", (unsigned long long)systime_ms());
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led_status();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led_status();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    print_led_status();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms = 0;
    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\r\n");
        return;
    }

    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\r\n");
        return;
    }

    print_led_status();
}

static void cmd_button(const command_t *command)
{
    (void)command;
    printf("button: %s, presses %u\r\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = PI_DEFAULT_TERMS;
    if (command->argc > 1)
    {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }
    if (command->argc == 1 && !parse_u32(command->argv[0], &terms))
    {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }
    if (terms == 0)
    {
        printf("error: terms must be greater than 0\r\n");
        return;
    }
    if (!pi_task_start(terms))
    {
        printf("error: pi is busy\r\n");
        return;
    }

    printf("pi: started, %u terms\r\n", (unsigned)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;
    double result = 0.0;
    uint64_t elapsed_ms = 0;

    switch (pi_task_get_state())
    {
    case PI_STATE_NOT_STARTED:
        printf("pi: not started\r\n");
        break;
    case PI_STATE_RUNNING:
        printf("pi: running\r\n");
        break;
    case PI_STATE_DONE:
        pi_task_get_result(&result, &elapsed_ms);
        printf("pi: %.9f in %llu ms\r\n", result, (unsigned long long)elapsed_ms);
        break;
    }
}

static void cmd_profiling(const command_t *command)
{
    (void)command;
#if PROFILING_ENABLED
    profiling_result_t res;
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        if (profiling_get(i, &res))
        {
            printf("%-11scount %-11umean %.2f us  max %u us\r\n",
                   res.name,
                   (unsigned)res.count,
                   res.mean_us,
                   (unsigned)res.max_us);
        }
    }
#else
    printf("profiling is disabled in this build\r\n");
#endif
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\r\n");
}

const api_command_t api_commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"led_enable", "turn led on", cmd_led_enable},
    {"led_disable", "turn led off", cmd_led_disable},
    {"led_blink", "blink led", cmd_led_blink},
    {"led_period", "set led blink period", cmd_led_period},
    {"button", "button state and press count", cmd_button},
    {"pi_start", "start pi calculation", cmd_pi_start},
    {"pi", "pi calculation state or result", cmd_pi},
    {"profiling", "show stopwatch results", cmd_profiling},
    {"profiling_reset", "reset stopwatch results", cmd_profiling_reset},
};

const uint32_t api_command_count = (uint32_t)(sizeof(api_commands) / sizeof(api_commands[0]));
