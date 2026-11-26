#include "pi-task/pi-task.h"

#include <stddef.h>
#include "systime/systime.h"

// На 125 МГц один член ряда Лейбница занимает ~585 тактов (~4.7 мкс).
// 100 членов за вызов занимают ~0.47 мс и укладываются в предел 0.5 мс.
#define PI_TERMS_PER_CALL 100U

static pi_state_t pi_state = PI_STATE_NOT_STARTED;
static uint32_t pi_total_terms = 0;
static uint32_t pi_k = 0;
static double pi_sum = 0.0;
static double pi_result = 0.0;
static uint64_t pi_start_ms = 0;
static uint64_t pi_elapsed_ms = 0;

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING || terms == 0)
    {
        return false;
    }

    pi_state = PI_STATE_RUNNING;
    pi_total_terms = terms;
    pi_k = 0;
    pi_sum = 0.0;
    pi_start_ms = systime_ms();
    pi_elapsed_ms = 0;
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING)
    {
        return;
    }

    uint32_t batch_end = pi_k + PI_TERMS_PER_CALL;
    if (batch_end < pi_k || batch_end > pi_total_terms)
    {
        batch_end = pi_total_terms;
    }

    for (; pi_k < batch_end; pi_k++)
    {
        double term = 1.0 / (2.0 * (double)pi_k + 1.0);
        if ((pi_k & 1U) != 0U)
        {
            pi_sum -= term;
        }
        else
        {
            pi_sum += term;
        }
    }

    if (pi_k >= pi_total_terms)
    {
        pi_result = 4.0 * pi_sum;
        pi_elapsed_ms = systime_ms() - pi_start_ms;
        pi_state = PI_STATE_DONE;
    }
}

pi_state_t pi_task_get_state(void)
{
    return pi_state;
}

bool pi_task_get_result(double *result, uint64_t *elapsed_ms)
{
    if (pi_state != PI_STATE_DONE)
    {
        return false;
    }
    if (result != NULL)
    {
        *result = pi_result;
    }
    if (elapsed_ms != NULL)
    {
        *elapsed_ms = pi_elapsed_ms;
    }
    return true;
}
