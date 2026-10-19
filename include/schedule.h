#ifndef SCHEDULE_H
#define SCHEDULE_H

/*
 * schedule - the weekly schedule: a fixed-capacity collection of events.
 *
 * The type is opaque (D8): clients never see its fields and read events through
 * sch_get. Only sch_new and sch_free allocate and release memory (D6). The
 * capacity is fixed when the schedule is created and a full schedule rejects
 * new events (D7).
 *
 * Ordering guarantee: events are kept sorted by day, then start, then end.
 * Events that are equal on all three keep their insertion order. sch_get(s, 0)
 * is therefore the earliest event of the week. The conflicts and freetime
 * modules rely on this guarantee.
 */

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"

typedef struct schedule schedule;

/*
 * sch_new - create an empty schedule that can hold up to `capacity` events.
 *
 * Parameters: capacity - maximum number of events (must be > 0); out - receives
 * the new schedule, to be released with sch_free.
 * Returns:
 *   AGENDA_OK             success.
 *   AGENDA_ERR_ARGUMENT   out is NULL or capacity is 0.
 *   AGENDA_ERR_NO_MEMORY  allocation failed.
 * *out is set to NULL on every error.
 */
agenda_status sch_new(size_t capacity, schedule **out);

/* sch_free - release a schedule. Accepts NULL (does nothing). */
void sch_free(schedule *s);

/*
 * sch_add - insert an event, keeping the ordering guarantee.
 *
 * Returns:
 *   AGENDA_OK            success.
 *   AGENDA_ERR_ARGUMENT  s or ev is NULL, or ev is not a valid event.
 *   AGENDA_ERR_FULL      the schedule already holds `capacity` events.
 * The event is validated before the capacity is checked. The schedule is
 * unchanged on error.
 */
agenda_status sch_add(schedule *s, const ts_event *ev);

/* sch_count - number of events stored. Returns 0 for a NULL schedule. */
size_t sch_count(const schedule *s);

/*
 * sch_get - copy the event at position `index` (0-based, in sorted order).
 *
 * Returns:
 *   AGENDA_OK            success; *out is written.
 *   AGENDA_ERR_ARGUMENT  s or out is NULL.
 *   AGENDA_ERR_RANGE     index >= sch_count(s).
 */
agenda_status sch_get(const schedule *s, size_t index, ts_event *out);

#endif /* SCHEDULE_H */
