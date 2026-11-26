#include "stdio-text-protocol/stdio-text-protocol.h"

#include <stddef.h>
#include <stdio.h>
#include "pico/stdlib.h"

static char line_buf[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_len = 0;
static bool line_overflow = false;
static command_t current_cmd;

static void reset_line(void)
{
    line_len = 0;
    line_overflow = false;
}

static bool parse_line(void)
{
    current_cmd.name = NULL;
    current_cmd.argc = 0;
    current_cmd.truncated = line_overflow;

    char *p = line_buf;
    while (*p != '\0')
    {
        while (*p == ' ')
        {
            *p = '\0';
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        const char *token = p;
        while (*p != '\0' && *p != ' ')
        {
            p++;
        }
        if (*p == ' ')
        {
            *p = '\0';
            p++;
        }

        if (current_cmd.name == NULL)
        {
            current_cmd.name = token;
        }
        else if (current_cmd.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
        {
            current_cmd.argv[current_cmd.argc++] = token;
        }
        else
        {
            current_cmd.truncated = true;
        }
    }

    return current_cmd.name != NULL;
}

void stdio_text_protocol_init(void)
{
    reset_line();
}

const command_t *stdio_text_protocol_handle(void)
{
    int ch = getchar_timeout_us(0);
    if (ch == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    if (ch == '\r' || ch == '\n')
    {
        if (line_len == 0 && !line_overflow)
        {
            return NULL;
        }
        printf("\r\n");
        line_buf[line_len] = '\0';
        bool has_cmd = parse_line();
        reset_line();
        return has_cmd ? &current_cmd : NULL;
    }

    if (ch == 0x08 || ch == 0x7f)
    {
        if (line_len > 0)
        {
            line_len--;
            line_overflow = false;
            printf("\b \b");
        }
        return NULL;
    }

    if (ch >= 0x20 && ch <= 0x7e)
    {
        if (line_len < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1)
        {
            line_buf[line_len++] = (char)ch;
            putchar(ch);
        }
        else
        {
            line_overflow = true;
        }
    }

    return NULL;
}
