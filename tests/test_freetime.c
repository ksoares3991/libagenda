#include "freetime.h"
#include "check.h"

static schedule *make(const ts_event *events, size_t n)
{
    schedule *s = NULL;
    size_t i;

    if (sch_new(n > 0 ? n : 1, &s) != AGENDA_OK) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        sch_add(s, &events[i]);
    }
    return s;
}

static void test_empty_day(void)
{
    schedule *s = make(NULL, 0);
    ts_event w[4];
    size_t count = 99;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 0, &busy), AGENDA_OK);
    CHECK_EQ(busy, 0);
    CHECK_EQ(ft_find_free(s, 0, w, 4, &count), AGENDA_OK);
    CHECK_EQ(count, 1);
    CHECK(w[0].day == 0 && w[0].start == 0 && w[0].end == 1440);
    sch_free(s);
}

static void test_whole_day_event(void)
{
    ts_event ev[] = { {2, 0, 1440} };
    schedule *s = make(ev, 1);
    ts_event w[2];
    size_t count = 7;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 2, &busy), AGENDA_OK);
    CHECK_EQ(busy, 1440);
    CHECK_EQ(ft_find_free(s, 2, w, 2, &count), AGENDA_OK);
    CHECK_EQ(count, 0);
    sch_free(s);
}

static void test_windows_between_events(void)
{

    ts_event ev[] = { {1, 720, 780}, {1, 480, 600} };
    schedule *s = make(ev, 2);
    ts_event w[4];
    size_t count = 0;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 1, &busy), AGENDA_OK);
    CHECK_EQ(busy, 180);
    CHECK_EQ(ft_find_free(s, 1, w, 4, &count), AGENDA_OK);
    CHECK_EQ(count, 3);
    CHECK(w[0].day == 1 && w[0].start == 0 && w[0].end == 480);
    CHECK(w[1].day == 1 && w[1].start == 600 && w[1].end == 720);
    CHECK(w[2].day == 1 && w[2].start == 780 && w[2].end == 1440);
    sch_free(s);
}

static void test_overlapping_events_count_once(void)
{

    ts_event ev[] = { {0, 480, 600}, {0, 540, 660}, {0, 500, 520} };
    schedule *s = make(ev, 3);
    ts_event w[4];
    size_t count = 0;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 0, &busy), AGENDA_OK);
    CHECK_EQ(busy, 180);

    CHECK_EQ(ft_find_free(s, 0, w, 4, &count), AGENDA_OK);
    CHECK_EQ(count, 2);
    CHECK(w[0].start == 0 && w[0].end == 480);
    CHECK(w[1].start == 660 && w[1].end == 1440);
    sch_free(s);
}

static void test_identical_events_count_once(void)
{
    ts_event ev[] = { {0, 540, 660}, {0, 540, 660}, {0, 540, 660} };
    schedule *s = make(ev, 3);
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 0, &busy), AGENDA_OK);
    CHECK_EQ(busy, 120);
    sch_free(s);
}

static void test_touching_events_leave_no_gap(void)
{
    ts_event ev[] = { {0, 480, 540}, {0, 540, 600} };
    schedule *s = make(ev, 2);
    ts_event w[4];
    size_t count = 0;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 0, &busy), AGENDA_OK);
    CHECK_EQ(busy, 120);
    CHECK_EQ(ft_find_free(s, 0, w, 4, &count), AGENDA_OK);
    CHECK_EQ(count, 2);
    CHECK(w[0].end == 480 && w[1].start == 600);
    sch_free(s);
}

static void test_events_at_the_day_edges(void)
{
    ts_event ev[] = { {0, 0, 60}, {0, 1380, 1440} };
    schedule *s = make(ev, 2);
    ts_event w[2];
    size_t count = 0;

    CHECK_EQ(ft_find_free(s, 0, w, 2, &count), AGENDA_OK);
    CHECK_EQ(count, 1);
    CHECK(w[0].start == 60 && w[0].end == 1380);
    sch_free(s);
}

static void test_other_days_are_ignored(void)
{
    ts_event ev[] = { {3, 480, 600} };
    schedule *s = make(ev, 1);
    ts_event w[2];
    size_t count = 0;
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 4, &busy), AGENDA_OK);
    CHECK_EQ(busy, 0);
    CHECK_EQ(ft_find_free(s, 4, w, 2, &count), AGENDA_OK);
    CHECK_EQ(count, 1);
    CHECK(w[0].day == 4 && w[0].start == 0 && w[0].end == 1440);
    CHECK_EQ(ft_total_busy(s, 3, &busy), AGENDA_OK);
    CHECK_EQ(busy, 120);
    sch_free(s);
}

static void test_busy_never_exceeds_a_day(void)
{
    ts_event ev[] = { {0, 0, 1440}, {0, 0, 1440}, {0, 100, 1000} };
    schedule *s = make(ev, 3);
    int busy = -1;

    CHECK_EQ(ft_total_busy(s, 0, &busy), AGENDA_OK);
    CHECK_EQ(busy, 1440);
    sch_free(s);
}

static void test_buffer_too_small(void)
{
    ts_event ev[] = { {0, 100, 200}, {0, 300, 400} };
    schedule *s = make(ev, 2);
    ts_event w[1];
    ts_event exact[3];
    size_t count = 0;

    CHECK_EQ(ft_find_free(s, 0, w, 1, &count), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(count, 3);
    CHECK(w[0].start == 0 && w[0].end == 100);

    count = 0;
    CHECK_EQ(ft_find_free(s, 0, NULL, 0, &count), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(count, 3);

    CHECK_EQ(ft_find_free(s, 0, exact, 3, &count), AGENDA_OK);
    CHECK_EQ(count, 3);
    CHECK(exact[2].start == 400 && exact[2].end == 1440);
    sch_free(s);
}

static void test_invalid_arguments(void)
{
    schedule *s = make(NULL, 0);
    ts_event w[1];
    size_t count;
    int busy;

    CHECK_EQ(ft_total_busy(s, -1, &busy), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_total_busy(s, TS_DAYS, &busy), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_find_free(s, -1, w, 1, &count), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_find_free(s, TS_DAYS, w, 1, &count), AGENDA_ERR_ARGUMENT);

    CHECK_EQ(ft_total_busy(NULL, 0, &busy), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_total_busy(s, 0, NULL), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_find_free(NULL, 0, w, 1, &count), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_find_free(s, 0, w, 1, NULL), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ft_find_free(s, 0, NULL, 1, &count), AGENDA_ERR_ARGUMENT);
    sch_free(s);
}

int main(void)
{
    test_empty_day();
    test_whole_day_event();
    test_windows_between_events();
    test_overlapping_events_count_once();
    test_identical_events_count_once();
    test_touching_events_leave_no_gap();
    test_events_at_the_day_edges();
    test_other_days_are_ignored();
    test_busy_never_exceeds_a_day();
    test_buffer_too_small();
    test_invalid_arguments();
    return check_summary("freetime");
}
