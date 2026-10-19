#include <string.h>
#include "freetime.h"

/* Marca cada minuto ocupado com 1; sobreposições não aumentam esse valor. */
static agenda_status mark_busy(const schedule *s, int day, unsigned char *busy)
{
    size_t n = sch_count(s);
    size_t i;
    ts_event ev;

    memset(busy, 0, TS_DAY_MINUTES);
    for (i = 0; i < n; i++) {
        if (sch_get(s, i, &ev) != AGENDA_OK) {
            return AGENDA_ERR_RANGE;
        }
        if (ev.day == day) {

            memset(busy + ev.start, 1, (size_t)(ev.end - ev.start));
        }
    }
    return AGENDA_OK;
}

agenda_status ft_total_busy(const schedule *s, int day, int *busy_minutes)
{
    unsigned char busy[TS_DAY_MINUTES];
    agenda_status rc;
    int minute;
    int total = 0;

    if (s == NULL || busy_minutes == NULL || day < 0 || day >= TS_DAYS) {
        return AGENDA_ERR_ARGUMENT;
    }
    rc = mark_busy(s, day, busy);
    if (rc != AGENDA_OK) {
        return rc;
    }
    for (minute = 0; minute < TS_DAY_MINUTES; minute++) {
        total += busy[minute];
    }
    *busy_minutes = total;
    return AGENDA_OK;
}

agenda_status ft_find_free(const schedule *s, int day, ts_event *buf, size_t cap, size_t *count)
{
    unsigned char busy[TS_DAY_MINUTES];
    agenda_status rc;
    size_t total = 0;
    int minute = 0;
    int window_start;

    if (s == NULL || count == NULL || (buf == NULL && cap > 0) ||
        day < 0 || day >= TS_DAYS) {
        return AGENDA_ERR_ARGUMENT;
    }
    rc = mark_busy(s, day, busy);
    if (rc != AGENDA_OK) {
        return rc;
    }

    while (minute < TS_DAY_MINUTES) {
        if (busy[minute]) {
            minute++;
            continue;
        }
        /* Cada sequência de minutos não marcados vira uma janela livre. */
        window_start = minute;
        while (minute < TS_DAY_MINUTES && !busy[minute]) {
            minute++;
        }
        if (total < cap) {
            buf[total].day = day;
            buf[total].start = window_start;
            buf[total].end = minute;
        }
        total++;
    }
    *count = total;
    return total > cap ? AGENDA_ERR_BUFFER_TOO_SMALL : AGENDA_OK;
}
