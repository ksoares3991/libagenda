#include "conflicts.h"
#include "check.h"

/* Build a schedule from a list of {day, start, end} events (times in minutes). */
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

static void test_has_conflict_none(void)
{
    int has = -1;
    schedule *s;
    ts_event one[] = { {0, 480, 580} };
    ts_event touching[] = { {0, 480, 580}, {0, 580, 600} };         /* 09:40 ends, 09:40 starts */
    ts_event days[] = { {0, 480, 580}, {1, 480, 580} };             /* same hours, different days */
    ts_event gap[] = { {0, 480, 500}, {0, 600, 700}, {0, 800, 900} };

    s = make(NULL, 0);                       /* empty schedule */
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 0);
    sch_free(s);

    s = make(one, 1);                        /* a single event */
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 0);
    sch_free(s);

    s = make(touching, 2);
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 0);
    sch_free(s);

    s = make(days, 2);
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 0);
    sch_free(s);

    s = make(gap, 3);
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 0);
    sch_free(s);
}

static void test_has_conflict_yes(void)
{
    int has = -1;
    schedule *s;
    ts_event partial[] = { {0, 480, 580}, {0, 560, 660} };          /* 20 minutes shared */
    ts_event inside[] = { {2, 480, 720}, {2, 540, 600} };
    ts_event same[] = { {4, 480, 580}, {4, 480, 580} };
    ts_event late[] = { {0, 100, 200}, {1, 100, 200}, {3, 500, 600}, {3, 599, 700} };

    s = make(partial, 2);
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 1);
    sch_free(s);

    s = make(inside, 2);
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 1);
    sch_free(s);

    s = make(same, 2);
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 1);
    sch_free(s);

    s = make(late, 4);                       /* the only conflict is in the last day with events */
    has = -1;
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 1);
    sch_free(s);
}

static void test_has_conflict_args(void)
{
    int has;
    schedule *s = make(NULL, 0);

    CHECK_EQ(cf_has_conflict(NULL, &has), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(cf_has_conflict(s, NULL), AGENDA_ERR_ARGUMENT);
    sch_free(s);
}

static void test_find_all(void)
{
    cf_pair pairs[8];
    size_t count = 99;
    schedule *s;
    /* After sorting: 0 MON 08:00-10:00, 1 MON 09:00-09:30, 2 MON 09:20-11:00, 3 TUE 08:00-09:00, 4 TUE 09:00-10:00 */
    ts_event ev[] = { {0, 480, 600}, {1, 480, 540}, {0, 540, 570}, {0, 560, 660}, {1, 540, 600} };

    /* No conflict: count 0, status OK. */
    s = make(NULL, 0);
    CHECK_EQ(cf_find_all(s, pairs, 8, &count), AGENDA_OK);
    CHECK_EQ(count, 0);
    sch_free(s);

    s = make(ev, 5);
    CHECK_EQ(cf_find_all(s, pairs, 8, &count), AGENDA_OK);
    /* Event 0 conflicts with 1 and 2 (one event in two conflicts); 1 conflicts with 2; TUE events only touch. */
    CHECK_EQ(count, 3);
    CHECK_EQ(pairs[0].first, 0);
    CHECK_EQ(pairs[0].second, 1);
    CHECK_EQ(pairs[1].first, 0);
    CHECK_EQ(pairs[1].second, 2);
    CHECK_EQ(pairs[2].first, 1);
    CHECK_EQ(pairs[2].second, 2);

    /* Buffer too small: first `cap` pairs are written, count says how many are needed. */
    pairs[0].first = 77;
    CHECK_EQ(cf_find_all(s, pairs, 2, &count), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(count, 3);
    CHECK_EQ(pairs[0].first, 0);
    CHECK_EQ(pairs[1].second, 2);

    /* cap 0 with NULL buffer is a valid way to ask only for the count. */
    CHECK_EQ(cf_find_all(s, NULL, 0, &count), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(count, 3);

    /* Invalid arguments. */
    CHECK_EQ(cf_find_all(NULL, pairs, 8, &count), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(cf_find_all(s, pairs, 8, NULL), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(cf_find_all(s, NULL, 4, &count), AGENDA_ERR_ARGUMENT);
    sch_free(s);
}

static void test_find_all_exact_capacity(void)
{
    cf_pair pairs[3];
    size_t count = 99;
    schedule *s;
    ts_event ev[] = { {0, 480, 600}, {0, 540, 570}, {0, 560, 660} };   /* three conflicting pairs */
    ts_event none[] = { {0, 480, 540}, {0, 540, 600} };                /* touching: no conflict */

    /* A buffer of exactly the needed size is not "too small". */
    s = make(ev, 3);
    CHECK_EQ(cf_find_all(s, pairs, 3, &count), AGENDA_OK);
    CHECK_EQ(count, 3);
    sch_free(s);

    /* No pairs and a zero-capacity buffer: nothing to report, so OK. */
    s = make(none, 2);
    count = 99;
    CHECK_EQ(cf_find_all(s, NULL, 0, &count), AGENDA_OK);
    CHECK_EQ(count, 0);
    sch_free(s);
}

static void test_nonadjacent_conflict_and_buffer_boundary(void)
{
    /* Insert out of order. The long event overlaps two separated short events. */
    ts_event events[] = { {0, 660, 720}, {0, 480, 780}, {0, 540, 600} };
    schedule *s = make(events, 3);
    cf_pair pairs[2] = { {99, 99}, {77, 88} };
    size_t count = 99;
    int has = -1;

    CHECK(s != NULL);
    if (s == NULL) {
        return;
    }
    CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
    CHECK_EQ(has, 1);
    CHECK_EQ(cf_find_all(s, pairs, 1, &count), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(count, 2);
    CHECK_EQ(pairs[0].first, 0);
    CHECK_EQ(pairs[0].second, 1);
    /* A second allocated entry exists but is outside the declared capacity. */
    CHECK_EQ(pairs[1].first, 77);
    CHECK_EQ(pairs[1].second, 88);
    CHECK_EQ(cf_find_all(s, pairs, 2, &count), AGENDA_OK);
    CHECK_EQ(count, 2);
    CHECK_EQ(pairs[0].first, 0);
    CHECK_EQ(pairs[0].second, 1);
    CHECK_EQ(pairs[1].first, 0);
    CHECK_EQ(pairs[1].second, 2);
    sch_free(s);
}

static void test_pairs_across_day_boundary(void)
{
    /* Identical hours on MON and SUN; only each day's own pair conflicts. */
    ts_event events[] = {
        {6, 480, 600}, {0, 540, 660}, {6, 540, 660}, {0, 480, 600}
    };
    schedule *s = make(events, 4);
    cf_pair pairs[6];
    size_t count = 99;

    CHECK(s != NULL);
    if (s == NULL) {
        return;
    }
    CHECK_EQ(cf_find_all(s, pairs, 6, &count), AGENDA_OK);
    CHECK_EQ(count, 2);
    CHECK_EQ(pairs[0].first, 0);
    CHECK_EQ(pairs[0].second, 1);
    CHECK_EQ(pairs[1].first, 2);
    CHECK_EQ(pairs[1].second, 3);
    sch_free(s);
}

int main(void)
{
    test_has_conflict_none();
    test_has_conflict_yes();
    test_has_conflict_args();
    test_find_all();
    test_find_all_exact_capacity();
    test_nonadjacent_conflict_and_buffer_boundary();
    test_pairs_across_day_boundary();
    return check_summary("conflicts");
}
