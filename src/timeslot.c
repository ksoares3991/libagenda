#include "timeslot.h"

#include <stdio.h>
#include <string.h>

/* Text layout: "MON 08:00-09:40" -> day(3) space(1) HH:MM(5) hyphen(1) HH:MM(5) = 15 characters. */
#define TEXT_LEN 15

static const char *const DAY_NAMES[TS_DAYS] = {
    "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"
};

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

/* Parse "HH:MM" at p (exactly five characters). Returns 1 and stores the minutes, or 0. */
static int parse_clock(const char *p, int *minutes)
{
    int hours;
    int mins;

    if (!is_digit(p[0]) || !is_digit(p[1]) || p[2] != ':' ||
        !is_digit(p[3]) || !is_digit(p[4])) {
        return 0;
    }
    hours = (p[0] - '0') * 10 + (p[1] - '0');
    mins = (p[3] - '0') * 10 + (p[4] - '0');
    if (mins > 59 || hours > 24 || (hours == 24 && mins != 0)) {
        return 0;
    }
    *minutes = hours * 60 + mins;
    return 1;
}

static int event_is_valid(const ts_event *ev)
{
    return ev != NULL &&
           ev->day >= 0 && ev->day < TS_DAYS &&
           ev->start >= 0 && ev->start < ev->end && ev->end <= TS_DAY_MINUTES;
}

agenda_status ts_parse(const char *text, ts_event *out)
{
    int day = -1;
    int start;
    int end;
    int d;

    if (text == NULL || out == NULL) {
        return AGENDA_ERR_ARGUMENT;
    }
    if (strlen(text) != TEXT_LEN || text[3] != ' ' || text[9] != '-') {
        return AGENDA_ERR_FORMAT;
    }
    for (d = 0; d < TS_DAYS; d++) {
        if (strncmp(text, DAY_NAMES[d], 3) == 0) {
            day = d;
            break;
        }
    }
    if (day < 0) {
        return AGENDA_ERR_FORMAT;
    }
    if (!parse_clock(text + 4, &start) || !parse_clock(text + 10, &end)) {
        return AGENDA_ERR_FORMAT;
    }
    if (end <= start) {
        return AGENDA_ERR_ARGUMENT;
    }
    out->day = day;
    out->start = start;
    out->end = end;
    return AGENDA_OK;
}

agenda_status ts_format(const ts_event *ev, char *buf, size_t cap)
{
    if (ev == NULL || buf == NULL || !event_is_valid(ev)) {
        return AGENDA_ERR_ARGUMENT;
    }
    if (cap < TS_TEXT_MAX) {
        if (cap > 0) {
            buf[0] = '\0';
        }
        return AGENDA_ERR_BUFFER_TOO_SMALL;
    }
    snprintf(buf, cap, "%s %02d:%02d-%02d:%02d",
             DAY_NAMES[ev->day],
             ev->start / 60, ev->start % 60,
             ev->end / 60, ev->end % 60);
    return AGENDA_OK;
}

agenda_status ts_duration(const ts_event *ev, int *minutes)
{
    if (minutes == NULL || !event_is_valid(ev)) {
        return AGENDA_ERR_ARGUMENT;
    }
    *minutes = ev->end - ev->start;
    return AGENDA_OK;
}

agenda_status ts_overlaps(const ts_event *a, const ts_event *b, int *minutes)
{
    int lo;
    int hi;

    if (minutes == NULL || !event_is_valid(a) || !event_is_valid(b)) {
        return AGENDA_ERR_ARGUMENT;
    }
    if (a->day != b->day) {
        *minutes = 0;
        return AGENDA_OK;
    }
    lo = a->start > b->start ? a->start : b->start;
    hi = a->end < b->end ? a->end : b->end;
    /* [start, end): events that only touch give hi == lo, hence 0 shared minutes. */
    *minutes = hi > lo ? hi - lo : 0;
    return AGENDA_OK;
}
