#include "schedule.h"

/* Stub: the header is frozen but the behaviour is not implemented yet. */

struct schedule {
    int unused;
};

agenda_status sch_new(size_t capacity, schedule **out)
{
    (void)capacity;
    if (out != NULL) {
        *out = NULL;
    }
    return AGENDA_ERR_ARGUMENT;
}

void sch_free(schedule *s)
{
    (void)s;
}

agenda_status sch_add(schedule *s, const ts_event *ev)
{
    (void)s;
    (void)ev;
    return AGENDA_ERR_ARGUMENT;
}

size_t sch_count(const schedule *s)
{
    (void)s;
    return 0;
}

agenda_status sch_get(const schedule *s, size_t index, ts_event *out)
{
    (void)s;
    (void)index;
    (void)out;
    return AGENDA_ERR_ARGUMENT;
}
