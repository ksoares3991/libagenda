#include "freetime.h"

/* Stub: the header is frozen but the behaviour is not implemented yet. */

agenda_status ft_find_free(const schedule *s, int day, ts_event *buf, size_t cap, size_t *count)
{
    (void)s;
    (void)day;
    (void)buf;
    (void)cap;
    (void)count;
    return AGENDA_ERR_ARGUMENT;
}

agenda_status ft_total_busy(const schedule *s, int day, int *busy)
{
    (void)s;
    (void)day;
    (void)busy;
    return AGENDA_ERR_ARGUMENT;
}
