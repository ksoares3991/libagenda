#include "conflicts.h"

/* Com a agenda ordenada, só há conflito se o próximo início vier antes do fim. */
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
    /* Basta comparar vizinhos para saber se existe pelo menos um conflito. */
    for (i = 0; i + 1 < n; i++) {
        if (sch_get(s, i, &ei) != AGENDA_OK || sch_get(s, i + 1, &ej) != AGENDA_OK) {
            return AGENDA_ERR_RANGE;
        }

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
            /* Depois deste ponto, nenhum evento seguinte pode conflitar com ei. */
            if (!conflicts_with_later(&ei, &ej)) {
                break;
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
