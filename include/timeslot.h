#ifndef TIMESLOT_H
#define TIMESLOT_H

/*
 * timeslot - one event of the weekly schedule: a day plus a time interval.
 *
 * Decisions (see README, "API decisions"):
 *  - D1: an event is a day of the week plus a start and an end time. Events
 *        only conflict when they are on the same day.
 *  - D2: times are minutes since 00:00, from 0 to 1440. Text uses HH:MM.
 *  - D3: the interval is [start, end): it includes the start and excludes the
 *        end, so events that merely touch do not overlap.
 *  - D4: invalid input is reported as an error, never silently "fixed".
 */

#include <stddef.h>
#include "agenda_error.h"

#define TS_DAYS        7     /* days of the week: 0 = MON ... 6 = SUN */
#define TS_DAY_MINUTES 1440  /* minutes in a day; the largest valid end time */
#define TS_TEXT_MAX    16    /* buffer size needed by ts_format: "MON 08:00-09:40" + '\0' */

/* A valid event satisfies 0 <= day < TS_DAYS and 0 <= start < end <= TS_DAY_MINUTES. */
typedef struct {
    int day;    /* 0 = MON, 1 = TUE, 2 = WED, 3 = THU, 4 = FRI, 5 = SAT, 6 = SUN */
    int start;  /* minutes since 00:00, inclusive */
    int end;    /* minutes since 00:00, exclusive */
} ts_event;

/*
 * ts_parse - convert text such as "MON 08:00-09:40" into an event.
 *
 * Text format: three-letter upper-case day (MON TUE WED THU FRI SAT SUN), one
 * space, start HH:MM, a hyphen, end HH:MM. HH goes from 00 to 24 and MM from
 * 00 to 59; 24:00 is accepted only as an end time ("24:00" with MM = 00).
 *
 * Parameters: text - NUL-terminated string; out - receives the event.
 * Returns:
 *   AGENDA_OK            success; *out is written.
 *   AGENDA_ERR_ARGUMENT  text or out is NULL, or end <= start.
 *   AGENDA_ERR_FORMAT    text does not match the format, the day name is
 *                        unknown, or a time is outside 00:00..24:00.
 * *out is not modified when an error is returned.
 */
agenda_status ts_parse(const char *text, ts_event *out);

/*
 * ts_format - write an event as text, using the format accepted by ts_parse.
 *
 * Parameters: ev - a valid event; buf - caller buffer; cap - size of buf in
 * bytes (TS_TEXT_MAX is always enough).
 * Returns:
 *   AGENDA_OK                   success; buf holds the NUL-terminated text.
 *   AGENDA_ERR_ARGUMENT         ev or buf is NULL, or ev is not a valid event.
 *   AGENDA_ERR_BUFFER_TOO_SMALL cap < TS_TEXT_MAX; if cap > 0, buf is set to
 *                               the empty string.
 */
agenda_status ts_format(const ts_event *ev, char *buf, size_t cap);

/*
 * ts_duration - duration of an event in minutes.
 *
 * Parameters: ev - a valid event; minutes - receives the duration (1..1440).
 * Returns: AGENDA_OK, or AGENDA_ERR_ARGUMENT if ev or minutes is NULL or ev is
 * not a valid event. This is also the way other modules validate an event.
 */
agenda_status ts_duration(const ts_event *ev, int *minutes);

/*
 * ts_overlaps - how many minutes two events share.
 *
 * Parameters: a, b - valid events; minutes - receives the shared minutes:
 * 0 means the events do not overlap (different days, or they only touch).
 * Example: MON 08:00-09:40 and MON 09:20-11:00 share 20 minutes; MON
 * 08:00-09:40 and MON 09:40-10:00 share 0.
 * Returns: AGENDA_OK, or AGENDA_ERR_ARGUMENT if any pointer is NULL or an event
 * is not valid.
 */
agenda_status ts_overlaps(const ts_event *a, const ts_event *b, int *minutes);

#endif /* TIMESLOT_H */
