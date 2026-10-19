#include "conflicts.h"

/* Stub: the header is frozen but the behaviour is not implemented yet. */

agenda_status cf_has_conflict(const schedule *s, int *has)
{
    (void)s;
    (void)has;
    return AGENDA_ERR_ARGUMENT;
}

agenda_status cf_find_all(const schedule *s, cf_pair *buf, size_t cap, size_t *count)
{
    (void)s;
    (void)buf;
    (void)cap;
    (void)count;
    return AGENDA_ERR_ARGUMENT;
}
