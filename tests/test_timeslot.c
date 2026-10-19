#include "timeslot.h"
#include "check.h"

#include <string.h>

static void test_parse_valid(void)
{
    ts_event e;

    CHECK_EQ(ts_parse("MON 08:00-09:40", &e), AGENDA_OK);
    CHECK_EQ(e.day, 0);
    CHECK_EQ(e.start, 480);
    CHECK_EQ(e.end, 580);

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

    CHECK_EQ(ts_parse("", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 8:00-09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("XYZ 08:00-09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("mon 08:00-09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 08:00 09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 08:00-09:4x", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 08:00-09:400", &e), AGENDA_ERR_FORMAT);

    CHECK_EQ(ts_parse("MON 08:60-09:40", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 25:00-26:00", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 23:00-24:01", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 23:00-25:00", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 25:00-25:30", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 99:99-99:99", &e), AGENDA_ERR_FORMAT);
    CHECK_EQ(ts_parse("MON 24:00-24:00", &e), AGENDA_ERR_ARGUMENT);

    CHECK_EQ(ts_parse("MON 09:40-08:00", &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_parse("MON 09:00-09:00", &e), AGENDA_ERR_ARGUMENT);

    CHECK_EQ(ts_parse(NULL, &e), AGENDA_ERR_ARGUMENT);
    CHECK_EQ(ts_parse("MON 08:00-09:40", NULL), AGENDA_ERR_ARGUMENT);

    CHECK_EQ(e.day, 99);
    CHECK_EQ(e.start, 99);
    CHECK_EQ(e.end, 99);
}

static void test_parse_checks_every_position(void)
{
    const char base[] = "MON 08:00-09:40";
    const char bad[] = { 'x', '/', ':' };
    ts_event e = {9, 9, 9};
    char text[sizeof base];
    size_t pos;
    size_t k;

    for (pos = 0; pos < sizeof base - 1; pos++) {
        for (k = 0; k < sizeof bad; k++) {
            memcpy(text, base, sizeof base);
            if (text[pos] == bad[k]) {
                continue;
            }
            text[pos] = bad[k];
            CHECK_EQ(ts_parse(text, &e), AGENDA_ERR_FORMAT);
        }
    }
    CHECK_EQ(e.day, 9);
    CHECK_EQ(ts_parse(base, &e), AGENDA_OK);
}

static void test_format(void)
{
    ts_event e;
    char buf[TS_TEXT_MAX];
    char small[8] = "xxxxxxx";
    const char *texts[] = { "MON 08:00-09:40", "SUN 00:00-24:00", "THU 13:05-13:06" };
    size_t i;

    for (i = 0; i < sizeof texts / sizeof texts[0]; i++) {
        CHECK_EQ(ts_parse(texts[i], &e), AGENDA_OK);
        CHECK_EQ(ts_format(&e, buf, sizeof buf), AGENDA_OK);
        CHECK_STREQ(buf, texts[i]);
    }

    e.day = 2; e.start = 60; e.end = 120;
    CHECK_EQ(ts_format(&e, buf, TS_TEXT_MAX), AGENDA_OK);
    CHECK_STREQ(buf, "WED 01:00-02:00");
    CHECK_EQ(ts_format(&e, buf, TS_TEXT_MAX - 1), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_STREQ(buf, "");
    CHECK_EQ(ts_format(&e, small, sizeof small), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_STREQ(small, "");
    buf[0] = 'X';
    CHECK_EQ(ts_format(&e, buf, 0), AGENDA_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ(buf[0], 'X');

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

    e.start = 1439; e.end = 1440;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_OK);
    CHECK_EQ(m, 1);

    e.start = 0; e.end = 1440;
    CHECK_EQ(ts_duration(&e, &m), AGENDA_OK);
    CHECK_EQ(m, 1440);

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
    ts_event a = {0, 480, 580};
    ts_event touching = {0, 580, 600};
    ts_event partial = {0, 560, 660};
    ts_event inside = {0, 500, 520};
    ts_event other_day = {1, 480, 580};
    ts_event bad = {0, 600, 600};
    int m = -1;

    CHECK_EQ(ts_overlaps(&a, &touching, &m), AGENDA_OK);
    CHECK_EQ(m, 0);
    CHECK_EQ(ts_overlaps(&touching, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 0);

    CHECK_EQ(ts_overlaps(&a, &partial, &m), AGENDA_OK);
    CHECK_EQ(m, 20);
    CHECK_EQ(ts_overlaps(&partial, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 20);

    CHECK_EQ(ts_overlaps(&a, &inside, &m), AGENDA_OK);
    CHECK_EQ(m, 20);

    CHECK_EQ(ts_overlaps(&a, &a, &m), AGENDA_OK);
    CHECK_EQ(m, 100);

    CHECK_EQ(ts_overlaps(&a, &other_day, &m), AGENDA_OK);
    CHECK_EQ(m, 0);

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

    CHECK_EQ(ts_overlaps(&early, &late, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 0);
    CHECK_EQ(ts_overlaps(&late, &early, &minutes), AGENDA_OK);
    CHECK_EQ(minutes, 0);

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
