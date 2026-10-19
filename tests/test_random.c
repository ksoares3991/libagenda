/*
 * test_random.c - randomized differential test across all four modules.
 *
 * The library results are compared with a brute-force "oracle" that is simple
 * enough to be obviously correct: a bitmap with one cell per minute of each day.
 *
 * Test parameters:
 *   GRANULARITY  every event starts at, and lasts, a multiple of 13 minutes.
 *                13 divides neither 60 nor 1440, so boundaries fall on odd
 *                minutes and the test cannot pass by only exercising round hours.
 *   SEED         seed of the pseudo-random generator, so every run is identical
 *                and a failure can be reproduced.
 *   EVENTS       number of events in each random schedule (and the schedule's
 *                capacity), dense enough to create many overlaps and conflicts.
 */

#include "conflicts.h"
#include "freetime.h"
#include "check.h"

#include <stdint.h>

#define GRANULARITY 13
#define SEED        2718281u
#define EVENTS      80
#define ROUNDS      60

static uint32_t rng_state = SEED;

/* Linear congruential generator: small, deterministic, no dependency on rand(). */
static uint32_t rng_next(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state >> 8;
}

static int rng_range(int lo, int hi)   /* inclusive */
{
    return lo + (int)(rng_next() % (uint32_t)(hi - lo + 1));
}

/* Oracle: how many minutes do two events share? (written without the library) */
static int shared_minutes(const ts_event *a, const ts_event *b)
{
    int minute;
    int shared = 0;

    if (a->day != b->day) {
        return 0;
    }
    for (minute = 0; minute < TS_DAY_MINUTES; minute++) {
        if (minute >= a->start && minute < a->end && minute >= b->start && minute < b->end) {
            shared++;
        }
    }
    return shared;
}

static void one_round(void)
{
    schedule *s = NULL;
    ts_event ev[EVENTS];
    static unsigned char busy_map[TS_DAYS][TS_DAY_MINUTES];
    static cf_pair pairs[EVENTS * EVENTS];
    static ts_event windows[TS_DAY_MINUTES];
    size_t pair_count = 0;
    size_t expected_pairs = 0;
    size_t i;
    size_t j;
    int day;
    int minute;

    CHECK_EQ(sch_new(EVENTS, &s), AGENDA_OK);
    for (day = 0; day < TS_DAYS; day++) {
        for (minute = 0; minute < TS_DAY_MINUTES; minute++) {
            busy_map[day][minute] = 0;
        }
    }

    for (i = 0; i < EVENTS; i++) {
        int start = GRANULARITY * rng_range(0, (TS_DAY_MINUTES - 1) / GRANULARITY);
        int end = start + GRANULARITY * rng_range(1, 10);

        if (end > TS_DAY_MINUTES) {
            end = TS_DAY_MINUTES;
        }
        ev[i].day = rng_range(0, TS_DAYS - 1);
        ev[i].start = start;
        ev[i].end = end;
        CHECK_EQ(sch_add(s, &ev[i]), AGENDA_OK);
        for (minute = start; minute < end; minute++) {
            busy_map[ev[i].day][minute] = 1;
        }
    }
    CHECK_EQ(sch_count(s), EVENTS);

    /* conflicts: the number of pairs that share at least one minute. */
    for (i = 0; i < EVENTS; i++) {
        for (j = i + 1; j < EVENTS; j++) {
            if (shared_minutes(&ev[i], &ev[j]) > 0) {
                expected_pairs++;
            }
        }
    }
    CHECK_EQ(cf_find_all(s, pairs, EVENTS * EVENTS, &pair_count), AGENDA_OK);
    CHECK_EQ(pair_count, expected_pairs);
    {
        int has = -1;

        CHECK_EQ(cf_has_conflict(s, &has), AGENDA_OK);
        CHECK_EQ(has, expected_pairs > 0);
    }
    /* Every reported pair must really conflict and be in increasing order. */
    for (i = 0; i < pair_count && i < expected_pairs; i++) {
        ts_event a;
        ts_event b;

        CHECK_EQ(sch_get(s, pairs[i].first, &a), AGENDA_OK);
        CHECK_EQ(sch_get(s, pairs[i].second, &b), AGENDA_OK);
        CHECK(pairs[i].first < pairs[i].second);
        CHECK(shared_minutes(&a, &b) > 0);
    }

    /* freetime: busy minutes and free windows must match the bitmap, day by day. */
    for (day = 0; day < TS_DAYS; day++) {
        int busy_expected = 0;
        int busy = -1;
        size_t win_count = 0;
        size_t w = 0;
        int in_free = 0;
        int free_start = 0;
        int ok = 1;

        for (minute = 0; minute < TS_DAY_MINUTES; minute++) {
            busy_expected += busy_map[day][minute];
        }
        CHECK_EQ(ft_total_busy(s, day, &busy), AGENDA_OK);
        CHECK_EQ(busy, busy_expected);

        CHECK_EQ(ft_find_free(s, day, windows, TS_DAY_MINUTES, &win_count), AGENDA_OK);
        /* Rebuild the windows from the bitmap and compare one by one. */
        for (minute = 0; minute <= TS_DAY_MINUTES; minute++) {
            int free_now = minute < TS_DAY_MINUTES && busy_map[day][minute] == 0;

            if (free_now && !in_free) {
                in_free = 1;
                free_start = minute;
            } else if (!free_now && in_free) {
                in_free = 0;
                if (w >= win_count || windows[w].start != free_start || windows[w].end != minute) {
                    ok = 0;
                }
                w++;
            }
        }
        CHECK(ok);
        CHECK_EQ(w, win_count);
    }

    sch_free(s);
}

int main(void)
{
    int round;

    for (round = 0; round < ROUNDS; round++) {
        one_round();
    }
    return check_summary("random");
}
