#include "schedule.h"

#include <stdint.h>
#include <stdlib.h>

/* Internal layout, hidden from clients (the type is opaque in schedule.h). */
struct schedule {
    size_t count;
    size_t capacity;
    ts_event *events;   /* sorted by (day, start, end); capacity entries allocated */
};

/* Returns 1 if a must come after b in the sorted order (strictly greater). */
static int is_after(const ts_event *a, const ts_event *b)
{
    if (a->day != b->day) {
        return a->day > b->day;
    }
    if (a->start != b->start) {
        return a->start > b->start;
    }
    return a->end > b->end;
}

agenda_status sch_new(size_t capacity, schedule **out)
{
    schedule *s;

    if (out == NULL) {
        return AGENDA_ERR_ARGUMENT;
    }
    *out = NULL;
    if (capacity == 0) {
        return AGENDA_ERR_ARGUMENT;
    }
    /* capacity * sizeof(ts_event) must not overflow size_t: refuse before allocating anything. */
    if (capacity > SIZE_MAX / sizeof(ts_event)) {
        return AGENDA_ERR_NO_MEMORY;
    }
    s = malloc(sizeof *s);
    if (s == NULL) {
        return AGENDA_ERR_NO_MEMORY;
    }
    s->events = calloc(capacity, sizeof *s->events);
    if (s->events == NULL) {
        free(s);
        return AGENDA_ERR_NO_MEMORY;
    }
    s->count = 0;
    s->capacity = capacity;
    *out = s;
    return AGENDA_OK;
}

void sch_free(schedule *s)
{
    if (s == NULL) {
        return;
    }
    free(s->events);
    free(s);
}

agenda_status sch_add(schedule *s, const ts_event *ev)
{
    int minutes;
    size_t pos;
    agenda_status rc;

    if (s == NULL || ev == NULL) {
        return AGENDA_ERR_ARGUMENT;
    }
    /* ts_duration is the validation entry point of the timeslot module. */
    rc = ts_duration(ev, &minutes);
    if (rc != AGENDA_OK) {
        return rc;
    }
    if (s->count == s->capacity) {
        return AGENDA_ERR_FULL;
    }
    /* Insertion sort step: shift later events one slot right. Equal keys keep insertion order. */
    pos = s->count;
    while (pos > 0 && is_after(&s->events[pos - 1], ev)) {
        s->events[pos] = s->events[pos - 1];
        pos--;
    }
    s->events[pos] = *ev;
    s->count++;
    return AGENDA_OK;
}

size_t sch_count(const schedule *s)
{
    return s == NULL ? 0 : s->count;
}

agenda_status sch_get(const schedule *s, size_t index, ts_event *out)
{
    if (s == NULL || out == NULL) {
        return AGENDA_ERR_ARGUMENT;
    }
    if (index >= s->count) {
        return AGENDA_ERR_RANGE;
    }
    *out = s->events[index];
    return AGENDA_OK;
}
