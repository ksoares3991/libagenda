#ifndef CONFLICTS_H
#define CONFLICTS_H

/*
 * conflicts - detect scheduling conflicts.
 *
 * Two events conflict when they are on the same day and share at least one
 * minute (D1, D3). Events that only touch (one ends exactly when the other
 * starts) do not conflict.
 */

#include <stddef.h>
#include "agenda_error.h"
#include "schedule.h"

/* A pair of conflicting events, given by their positions in the schedule (sch_get order). */
typedef struct {
    size_t first;   /* position of the event that comes first; first < second */
    size_t second;  /* position of the event that comes later */
} cf_pair;

/*
 * cf_has_conflict - tell whether the schedule has at least one conflict.
 *
 * Parameters: s - the schedule; has - receives 1 if there is a conflict, else 0.
 * Returns: AGENDA_OK, or AGENDA_ERR_ARGUMENT if s or has is NULL.
 * An empty schedule, or one with a single event, has no conflict.
 */
agenda_status cf_has_conflict(const schedule *s, int *has);

/*
 * cf_find_all - list every pair of conflicting events.
 *
 * Pairs are written to the caller's buffer (D6) in order: by the position of
 * the first event, then by the position of the second. Since the schedule is
 * sorted by day and start, this is also the order by day and start time.
 *
 * Parameters: s - the schedule; buf - caller buffer for up to `cap` pairs (may
 * be NULL only if cap is 0); cap - capacity of buf in pairs; count - receives
 * the TOTAL number of conflicting pairs, even when they do not all fit.
 * Returns:
 *   AGENDA_OK                   all pairs were written; *count <= cap.
 *   AGENDA_ERR_ARGUMENT         s or count is NULL, or buf is NULL with cap > 0.
 *   AGENDA_ERR_BUFFER_TOO_SMALL more than `cap` pairs exist: the first `cap`
 *                               pairs are written and *count is the number
 *                               needed, so the caller can retry with a bigger buffer.
 */
agenda_status cf_find_all(const schedule *s, cf_pair *buf, size_t cap, size_t *count);

#endif /* CONFLICTS_H */
