#include "timeslot.h"

/* Stub: the header is frozen but the behaviour is not implemented yet. */

agenda_status ts_parse(const char *text, ts_event *out)
{
    (void)text;
    (void)out;
    return AGENDA_ERR_ARGUMENT;
}

agenda_status ts_format(const ts_event *ev, char *buf, size_t cap)
{
    (void)ev;
    (void)buf;
    (void)cap;
    return AGENDA_ERR_ARGUMENT;
}

agenda_status ts_duration(const ts_event *ev, int *minutes)
{
    (void)ev;
    (void)minutes;
    return AGENDA_ERR_ARGUMENT;
}

agenda_status ts_overlaps(const ts_event *a, const ts_event *b, int *minutes)
{
    (void)a;
    (void)b;
    (void)minutes;
    return AGENDA_ERR_ARGUMENT;
}
