#include "schedule.h"
#include "check.h"

#include <stdint.h>

static void test_new_and_free(void)
{
    schedule *s = (schedule *)0x1;   /* must be overwritten */

    CHECK_EQ(sch_new(4, &s), AGENDA_OK);
    CHECK(s != NULL);
    CHECK_EQ(sch_count(s), 0);       /* a new schedule is empty */
    sch_free(s);

    sch_free(NULL);                  /* freeing NULL is allowed */

    /* Invalid arguments: capacity 0 and NULL output. *out becomes NULL. */
    s = (schedule *)0x1;
    CHECK_EQ(sch_new(0, &s), AGENDA_ERR_ARGUMENT);
    CHECK(s == NULL);
    CHECK_EQ(sch_new(4, NULL), AGENDA_ERR_ARGUMENT);

    /* An impossible capacity makes the allocation fail instead of crashing. */
    s = (schedule *)0x1;
    CHECK_EQ(sch_new(SIZE_MAX / 2, &s), AGENDA_ERR_NO_MEMORY);
    CHECK(s == NULL);

    CHECK_EQ(sch_count(NULL), 0);
}

static void test_add_until_full(void)
{
    schedule *s;
    ts_event e = {0, 480, 580};
    ts_event bad = {0, 600, 600};
    ts_event got;

    CHECK_EQ(sch_new(2, &s), AGENDA_OK);

    CHECK_EQ(sch_add(s, &e), AGENDA_OK);
    CHECK_EQ(sch_count(s), 1);
    e.day = 1;
    CHECK_EQ(sch_add(s, &e), AGENDA_OK);   /* adding exactly up to the capacity works */
    CHECK_EQ(sch_count(s), 2);

    CHECK_EQ(sch_add(s, &e), AGENDA_ERR_FULL);   /* one more does not */
    CHECK_EQ(sch_count(s), 2);

    /* An invalid event is rejected and never stored; validity is checked before capacity. */
    CHECK_EQ(sch_add(s, &bad), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_count(s), 2);

    /* NULL pointers. */
    CHECK_EQ(sch_add(NULL, &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_add(s, NULL), AGENDA_ERR_ARGUMENT);

    /* The stored events were not corrupted by the failed calls. */
    CHECK_EQ(sch_get(s, 0, &got), AGENDA_OK);
    CHECK_EQ(got.day, 0);
    CHECK_EQ(sch_get(s, 1, &got), AGENDA_OK);
    CHECK_EQ(got.day, 1);

    sch_free(s);
}

static void test_get(void)
{
    schedule *s;
    ts_event e = {2, 60, 120};
    ts_event got = {9, 9, 9};

    CHECK_EQ(sch_new(3, &s), AGENDA_OK);

    /* Empty schedule: every index is out of range. */
    CHECK_EQ(sch_get(s, 0, &got), AGENDA_ERR_RANGE);

    CHECK_EQ(sch_add(s, &e), AGENDA_OK);
    CHECK_EQ(sch_get(s, 0, &got), AGENDA_OK);
    CHECK_EQ(got.day, 2);
    CHECK_EQ(got.start, 60);
    CHECK_EQ(got.end, 120);

    /* Index equal to the count, and far beyond, are out of range. */
    CHECK_EQ(sch_get(s, 1, &got), AGENDA_ERR_RANGE);
    CHECK_EQ(sch_get(s, SIZE_MAX, &got), AGENDA_ERR_RANGE);

    CHECK_EQ(sch_get(NULL, 0, &got), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_get(s, 0, NULL), AGENDA_ERR_ARGUMENT);

    sch_free(s);
}

static void test_allocation_failure(void)
{
#if !defined(__SANITIZE_ADDRESS__)
    /* The size fits in size_t (no overflow) but no machine has that much memory: malloc fails. */
    schedule *s = (schedule *)0x1;

    CHECK_EQ(sch_new(SIZE_MAX / 16, &s), AGENDA_ERR_NO_MEMORY);
    CHECK(s == NULL);
#endif
    /* Under AddressSanitizer a huge malloc aborts instead of returning NULL, so it is skipped there. */
}

static void test_sorted_order(void)
{
    schedule *s;
    ts_event in[] = {
        {3, 600, 700},   /* THU 10:00 */
        {0, 900, 960},   /* MON 15:00 */
        {0, 480, 580},   /* MON 08:00-09:40 */
        {0, 480, 520},   /* MON 08:00-08:40: same start, shorter end -> first */
        {6, 0, 1440}     /* SUN whole day */
    };
    int want_day[] = {0, 0, 0, 3, 6};
    int want_start[] = {480, 480, 900, 600, 0};
    int want_end[] = {520, 580, 960, 700, 1440};
    size_t i;
    ts_event got;

    CHECK_EQ(sch_new(5, &s), AGENDA_OK);
    for (i = 0; i < 5; i++) {
        CHECK_EQ(sch_add(s, &in[i]), AGENDA_OK);
    }
    /* sch_get returns events by day, then start, then end, whatever the insertion order. */
    for (i = 0; i < 5; i++) {
        CHECK_EQ(sch_get(s, i, &got), AGENDA_OK);
        CHECK_EQ(got.day, want_day[i]);
        CHECK_EQ(got.start, want_start[i]);
        CHECK_EQ(got.end, want_end[i]);
    }
    sch_free(s);
}

static void test_add_rejects_each_invalid_field(void)
{
    schedule *s;
    ts_event ok = {2, 0, 1440};                  /* the widest valid event */
    ts_event bad[] = {
        {-1, 0, 60},                             /* day below 0 */
        {TS_DAYS, 0, 60},                        /* day above SUN */
        {0, -1, 60},                             /* negative start */
        {0, 600, 1441},                          /* end after 24:00 */
        {0, 700, 600},                           /* end before start */
        {0, 600, 600}                            /* empty interval */
    };
    size_t i;

    CHECK_EQ(sch_new(8, &s), AGENDA_OK);
    for (i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        CHECK_EQ(sch_add(s, &bad[i]), AGENDA_ERR_ARGUMENT);
    }
    CHECK_EQ(sch_count(s), 0);                   /* nothing invalid was stored */
    CHECK_EQ(sch_add(s, &ok), AGENDA_OK);        /* boundary values are valid */
    CHECK_EQ(sch_count(s), 1);
    sch_free(s);
}

static void test_add_stores_a_copy(void)
{
    schedule *s;
    ts_event e = {0, 60, 120};
    ts_event got;

    CHECK_EQ(sch_new(1, &s), AGENDA_OK);
    CHECK_EQ(sch_add(s, &e), AGENDA_OK);
    e.start = 0;                                 /* the caller reuses its variable */
    e.end = 1;
    CHECK_EQ(sch_get(s, 0, &got), AGENDA_OK);
    CHECK_EQ(got.start, 60);
    CHECK_EQ(got.end, 120);
    sch_free(s);
}

int main(void)
{
    test_new_and_free();
    test_add_until_full();
    test_get();
    test_allocation_failure();
    test_sorted_order();
    test_add_rejects_each_invalid_field();
    test_add_stores_a_copy();
    return check_summary("schedule");
}
