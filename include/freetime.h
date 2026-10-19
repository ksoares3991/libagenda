#ifndef FREETIME_H
#define FREETIME_H

/*
 * freetime - free windows and busy time of one day of the schedule.
 *
 * Overlapping events are merged: the time they share is counted once and never
 * creates a false free window (D3: intervals are [start, end)).
 */

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"
#include "schedule.h"

/*
 * ft_find_free - list the free windows of one day, ordered by start time.
 *
 * Each window is written as a ts_event: the requested day plus the window's
 * start and end (so it can be printed with ts_format). Events that overlap or
 * only touch never create a false window.
 *
 * Parameters: s - the schedule; day - 0 (MON) to 6 (SUN); buf - caller buffer
 * for up to `cap` windows (may be NULL only if cap is 0); cap - capacity of buf
 * in windows; count - receives the TOTAL number of windows, even if they do not
 * all fit.
 * An empty day is one window, 00:00-24:00. A day fully covered by events has
 * no window.
 * Returns:
 *   AGENDA_OK                   all windows were written; *count <= cap.
 *   AGENDA_ERR_ARGUMENT         s or count is NULL, day is outside 0..6, or buf
 *                               is NULL with cap > 0.
 *   AGENDA_ERR_BUFFER_TOO_SMALL more than `cap` windows exist: the first `cap`
 *                               are written and *count is the number needed.
 */
agenda_status ft_find_free(const schedule *s, int day, ts_event *buf, size_t cap, size_t *count);

/*
 * ft_total_busy - minutes of one day covered by at least one event.
 *
 * Time shared by overlapping events is counted once, so the result is between
 * 0 (empty day) and 1440.
 * Returns: AGENDA_OK, or AGENDA_ERR_ARGUMENT if s or busy is NULL or day is
 * outside 0..6.
 */
agenda_status ft_total_busy(const schedule *s, int day, int *busy);

#endif /* FREETIME_H */
