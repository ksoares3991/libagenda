#include "timeslot.h"
#include "check.h"

#include <string.h>

/* Events are written as {day, start, end} with times in minutes: 08:00 = 480, 09:40 = 580. */

static void test_parse_valid(void)
{
    ts_event e;

    CHECK_EQ(ts_parse("MON 08:00-09:40", &e), AGENDA_OK);
    CHECK_EQ(e.day, 0);
    CHECK_EQ(e.start, 480);
    CHECK_EQ(e.end, 580);

    /* Limits: whole day, last day of the week, shortest possible event (1 minute). */
    CHECK_EQ(ts_parse("SUN 00:00-24:00", &e), AGENDA_OK);
    CHECK_EQ(e.day, 6);
    CHECK_EQ(e.start, 0);
    CHECK_EQ(e.end, 1440);
    CHECK_EQ(ts_parse("FRI 23:59-24:00", &e), AGENDA_OK);
    CHECK_EQ(e.start, 1439);
    CHECK_EQ(e.end, 1440);
}

static void test_parse_invalid(void)
{
    ts_event e = {99, 99, 99};

    /* Malformed text -> AGENDA_ERR_FORMAT. */
    CHECK_EQ(ts_parse("", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 8:00-09:40", &e), AGENDA_ERR_FORMAT);    /* one-digit hour */
    CHECK_EQ(ts_parse("XYZ 08:00-09:40", &e), AGENDA_ERR_FORMAT);   /* unknown day */
    CHECK_EQ(ts_parse("mon 08:00-09:40", &e), AGENDA_ERR_FORMAT);   /* lower case */
    CHECK_EQ(ts_parse("MON 08:00 09:40", &e), AGENDA_ERR_FORMAT);   /* wrong separator */
    CHECK_EQ(ts_parse("MON 08:00-09:4x", &e), AGENDA_ERR_FORMAT);   /* not a digit */
    CHECK_EQ(ts_parse("MON 08:00-09:400", &e), AGENDA_ERR_FORMAT);  /* too long */

    /* Times outside 00:00..24:00 -> AGENDA_ERR_FORMAT. */
    CHECK_EQ(ts_parse("MON 08:60-09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 25:00-26:00", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 23:00-24:01", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 23:00-25:00", &e), AGENDA_ERR_FORMAT);   /* end beyond 24:00 */
    CHECK_EQ(ts_parse("MON 25:00-25:30", &e), AGENDA_ERR_FORMAT);   /* both beyond 24:00 */
    CHECK_EQ(ts_parse("MON 99:99-99:99", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 24:00-24:00", &e), AGENDA_ERR_ARGUMENT); /* valid times, empty interval */

    /* Valid text but end <= start -> AGENDA_ERR_ARGUMENT. */
    CHECK_EQ(ts_parse("MON 09:40-08:00", &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_parse("MON 09:00-09:00", &e), AGENDA_ERR_ARGUMENT);

    /* NULL pointers. */
    CHECK_EQ(ts_parse(NULL, &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_parse("MON 08:00-09:40", NULL), AGENDA_ERR_ARGUMENT);

    /* The output is left untouched on error. */
    CHECK_EQ(e.day, 99);
    CHECK_EQ(e.start, 99);
    CHECK_EQ(e.end, 99);
}

static void test_parse_checks_every_position(void)
{
    const char base[] = "MON 08:00-09:40";
    const char bad[] = { 'x', '/', ':' };   /* '/' is just below '0' and ':' just above '9' */
    ts_event e = {9, 9, 9};
    char text[sizeof base];
    size_t pos;
    size_t k;

    /* Replacing any single character by a wrong one must be rejected. */
    for (pos = 0; pos < sizeof base - 1; pos++) {
        for (k = 0; k < sizeof bad; k++) {
            memcpy(text, base, sizeof base);
            if (text[pos] == bad[k]) {
                continue;                   /* unchanged: the text is still valid */
            }
            text[pos] = bad[k];
            CHECK_EQ(ts_parse(text, &e), AGENDA_ERR_FORMAT);
        }
    }
    CHECK_EQ(e.day, 9);                     /* *out is untouched by every failure */
    CHECK_EQ(ts_parse(base, &e), AGENDA_OK);
}

static void test_format(void)
{
    ts_event e;
    char buf[TS_TEXT_MAX];
    char small[8] = "xxxxxxx";
    const char *texts[] = { "MON 08:00-09:40", "SUN 00:00-24:00", "THU 13:05-13:06" };
    size_t i;

    /* ts_parse followed by ts_format returns the original text. */
    for (i = 0; i < sizeof texts / sizeof texts[0]; i++) {
        CHECK_EQ(ts_parse(texts[i], &e), AGENDA_OK);
        CHECK_EQ(ts_format(&e, buf, sizeof buf), AGENDA_OK);
        CHECK_STREQ(buf, texts[i]);
    }

    /* Exact-fit buffer works; one byte less does not. */
    e.day = 2; e.start = 60; e.end = 120;
    CHECK_EQ(ts_format(&e, buf, TS_TEXT_MAX), AGENDA_OK);
    CHECK_STREQ(buf, "WED 01:00-02:00");
    CHECK_EQ(ts_format(&e, buf, TS_TEXT_MAX - 1), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_STREQ(buf, "");
    CHECK_EQ(ts_format(&e, small, sizeof small), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_STREQ(small, "");
    buf[0] = 'X';
    CHECK_EQ(ts_format(&e, buf, 0), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(buf[0], 'X');                   /* with cap 0 nothing may be written, not even '\0' */

    /* Invalid input. */
    CHECK_EQ(ts_format(NULL, buf, sizeof buf), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_format(&e, NULL, 32), AGENDA_ERR_ARGUMENT);
    e.end = e.start;
    CHECK_EQ(ts_format(&e, buf, sizeof buf), AGENDA_ERR_ARGUMENT);
    e.start = 60; e.end = 120; e.day = 7;
    CHECK_EQ(ts_format(&e, buf, sizeof buf), AGENDA_ERR_ARGUMENT);
}

static void test_duration(void)
{
    ts_event e = {0, 480, 580};
    int m = -1;

    CHECK_EQ(ts_duration(&e, &m), AGENDA_OK);
    CHECK_EQ(m, 100);

    e.start = 1439; e.end = 1440;   /* shortest event: 1 minute */
    CHECK_EQ(ts_duration(&e, &m), AGENDA_OK);
    CHECK_EQ(m, 1);

    e.start = 0; e.end = 1440;      /* whole day */
    CHECK_EQ(ts_duration(&e, &m), AGENDA_OK);
    CHECK_EQ(m, 1440);

    /* Invalid events and NULL pointers. */
    e.start = 500; e.end = 500;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_ERR_ARGUMENT);
    e.start = 0; e.end = 1441;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_ERR_ARGUMENT);
    e.start = -1; e.end = 10;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_ERR_ARGUMENT);
    e.start = 0; e.end = 10; e.day = -1;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_duration(NULL, &m), AGENDA_ERR_ARGUMENT);
    e.day = 0;
    CHECK_EQ(ts_duration(&e, NULL), AGENDA_ERR_ARGUMENT);
}

static void test_overlaps(void)
{
    ts_event a = {0, 480, 580};        /* MON 08:00-09:40 */
    ts_event touching = {0, 580, 600}; /* MON 09:40-10:00 */
    ts_event partial = {0, 560, 660};  /* MON 09:20-11:00 */
    ts_event inside = {0, 500, 520};   /* MON 08:20-08:40 */
    ts_event other_day = {1, 480, 580};
    ts_event bad = {0, 600, 600};
    int m = -1;

    /* Only touching: [start, end) means no shared minute. */
    CHECK_EQ(ts_overlaps(&a, &touching, &m), AGENDA_OK);
    CHECK_EQ(m, 0);
    CHECK_EQ(ts_overlaps(&touching, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 0);

    /* Partial overlap: 20 minutes, in both argument orders. */
    CHECK_EQ(ts_overlaps(&a, &partial, &m), AGENDA_OK);
    CHECK_EQ(m, 20);
    CHECK_EQ(ts_overlaps(&partial, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 20);

    /* One inside the other: the shorter event is entirely shared. */
    CHECK_EQ(ts_overlaps(&a, &inside, &m), AGENDA_OK);
    CHECK_EQ(m, 20);

    /* Identical events share their whole duration. */
    CHECK_EQ(ts_overlaps(&a, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 100);

    /* Same hours on different days never overlap. */
    CHECK_EQ(ts_overlaps(&a, &other_day, &m), AGENDA_OK);
    CHECK_EQ(m, 0);

    /* Invalid events and NULL pointers. */
    CHECK_EQ(ts_overlaps(&a, &bad, &m), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_overlaps(&bad, &a, &m), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_overlaps(NULL, &a, &m), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_overlaps(&a, NULL, &m), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_overlaps(&a, &a, NULL), AGENDA_ERR_ARGUMENT);
}

static void test_sunday_last_hour(void)
{
    ts_event e = {-1, -1, -1};
    char text[TS_TEXT_MAX];
    int minutes = -1;

    CHECK_EQ(ts_parse("SUN 23:00-24:00", &e), AGENDA_OK);
    CHECK_EQ(e.day, 6);
    CHECK_EQ(e.start, 1380);
    CHECK_EQ(e.end, 1440);
    CHECK_EQ(ts_duration(&e, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 60);
    CHECK_EQ(ts_format(&e, text, sizeof text), AGENDA_OK);
    CHECK_STREQ(text, "SUN 23:00-24:00");
}

static void test_separated_and_week_boundary(void)
{
    ts_event early = {0, 0, 1};
    ts_event late = {0, 1439, 1440};
    ts_event sunday = {6, 1439, 1440};
    int minutes = -1;

    /* Disjoint intervals on the same day, in both argument orders. */
    CHECK_EQ(ts_overlaps(&early, &late, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 0);
    CHECK_EQ(ts_overlaps(&late, &early, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 0);
    /* The last minute on Sunday does not overlap the last minute on Monday. */
    CHECK_EQ(ts_overlaps(&sunday, &late, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 0);
}

int main(void)
{
    test_parse_valid();
    test_parse_invalid();
    test_parse_checks_every_position();
    test_format();
    test_duration();
    test_overlaps();
    test_sunday_last_hour();
    test_separated_and_week_boundary();
    return check_summary("timeslot");
}
