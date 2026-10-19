#include "schedule.h"
#include "check.h"

#include <stdint.h>

static void test_new_and_free(void)
{
    schedule *s = (schedule *)0x1;

    CHECK_EQ(sch_new(4, &s), AGENDA_OK);
    CHECK(s != NULL);
    CHECK_EQ(sch_count(s), 0);
    sch_free(s);

    sch_free(NULL);

    s = (schedule *)0x1;
    CHECK_EQ(sch_new(0, &s), AGENDA_ERR_ARGUMENT);
    CHECK(s == NULL);
    CHECK_EQ(sch_new(4, NULL), AGENDA_ERR_ARGUMENT);

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
    CHECK_EQ(sch_add(s, &e), AGENDA_OK);
    CHECK_EQ(sch_count(s), 2);

    CHECK_EQ(sch_add(s, &e), AGENDA_ERR_FULL);
    CHECK_EQ(sch_count(s), 2);

    CHECK_EQ(sch_add(s, &bad), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_count(s), 2);

    CHECK_EQ(sch_add(NULL, &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_add(s, NULL), AGENDA_ERR_ARGUMENT);

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

    CHECK_EQ(sch_get(s, 0, &got), AGENDA_ERR_RANGE);

    CHECK_EQ(sch_add(s, &e), AGENDA_OK);
    CHECK_EQ(sch_get(s, 0, &got), AGENDA_OK);
    CHECK_EQ(got.day, 2);
    CHECK_EQ(got.start, 60);
    CHECK_EQ(got.end, 120);

    CHECK_EQ(sch_get(s, 1, &got), AGENDA_ERR_RANGE);
    CHECK_EQ(sch_get(s, SIZE_MAX, &got), AGENDA_ERR_RANGE);

    CHECK_EQ(sch_get(NULL, 0, &got), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(sch_get(s, 0, NULL), AGENDA_ERR_ARGUMENT);

    sch_free(s);
}

static void test_allocation_failure(void)
{
/* O ASan pode abortar um pedido enorme de memória, em vez de devolver NULL. */
#if !defined(__SANITIZE_ADDRESS__)

    schedule *s = (schedule *)0x1;

    CHECK_EQ(sch_new(SIZE_MAX / 16, &s), AGENDA_ERR_NO_MEMORY);
    CHECK(s == NULL);
#endif

}

static void test_sorted_order(void)
{
    schedule *s;
    ts_event in[] = {
        {3, 600, 700},
        {0, 900, 960},
        {0, 480, 580},
        {0, 480, 520},
        {6, 0, 1440}
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
    ts_event ok = {2, 0, 1440};
    ts_event bad[] = {
        {-1, 0, 60},
        {TS_DAYS, 0, 60},
        {0, -1, 60},
        {0, 600, 1441},
        {0, 700, 600},
        {0, 600, 600}
    };
    size_t i;

    CHECK_EQ(sch_new(8, &s), AGENDA_OK);
    for (i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        CHECK_EQ(sch_add(s, &bad[i]), AGENDA_ERR_ARGUMENT);
    }
    CHECK_EQ(sch_count(s), 0);
    CHECK_EQ(sch_add(s, &ok), AGENDA_OK);
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
    e.start = 0;
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
