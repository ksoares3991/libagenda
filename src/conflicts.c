#include "conflicts.h"

/*
 * The schedule is sorted by (day, start, end), so for an event i the events
 * that can conflict with it are the ones right after it. The scan for i stops
 * at the first later event that is on another day or starts at or after the end
 * of i: every event after that one starts even later and cannot overlap i.
 * Tracking this makes the scan O(n + number of conflicts) instead of O(n^2).
 */

/*
 * ej comes after ei in sorted order (so ej.start >= ei.start when on the same
 * day). They conflict iff ej is on the same day and starts before ei ends.
 * A 0 result also means "no later event can conflict with ei": the scan stops.
 */
static int conflicts_with_later(const ts_event *ei, const ts_event *ej)
{
    return ej->day == ei->day && ej->start < ei->end;
}

agenda_status cf_has_conflict(const schedule *s, int *has)
{
    size_t n;
    size_t i;
    ts_event ei;
    ts_event ej;

    if (s == NULL || has == NULL) {
        return AGENDA_ERR_ARGUMENT;
    }
    *has = 0;
    n = sch_count(s);
    for (i = 0; i + 1 < n; i++) {
        if (sch_get(s, i, &ei) != AGENDA_OK || sch_get(s, i + 1, &ej) != AGENDA_OK) {
            return AGENDA_ERR_RANGE;
        }
        /* If the very next event does not conflict with i, none of the later ones does. */
        if (conflicts_with_later(&ei, &ej)) {
            *has = 1;
            return AGENDA_OK;
        }
    }
    return AGENDA_OK;
}

agenda_status cf_find_all(const schedule *s, cf_pair *buf, size_t cap, size_t *count)
{
    size_t n;
    size_t i;
    size_t j;
    size_t total = 0;
    ts_event ei;
    ts_event ej;

    if (s == NULL || count == NULL || (buf == NULL && cap > 0)) {
        return AGENDA_ERR_ARGUMENT;
    }
    n = sch_count(s);
    for (i = 0; i < n; i++) {
        if (sch_get(s, i, &ei) != AGENDA_OK) {
            return AGENDA_ERR_RANGE;
        }
        for (j = i + 1; j < n; j++) {
            if (sch_get(s, j, &ej) != AGENDA_OK) {
                return AGENDA_ERR_RANGE;
            }
            if (!conflicts_with_later(&ei, &ej)) {
                break;   /* sorted order: later events cannot conflict with i either */
            }
            if (total < cap) {
                buf[total].first = i;
                buf[total].second = j;
            }
            total++;
        }
    }
    *count = total;
    return total > cap ? AGENDA_ERR_BUFFER_TOO_SMALL : AGENDA_OK;
}
