#include <stdio.h>
#include <string.h>

#include "agenda_error.h"
#include "conflicts.h"
#include "freetime.h"
#include "schedule.h"
#include "timeslot.h"

#define CAPACITY 64
#define MAX_PAIRS 256
/* N eventos separados podem deixar N + 1 janelas livres. */
#define MAX_WINDOWS (CAPACITY + 1)

static const char *status_text(agenda_status rc)
{
    switch (rc) {
    case AGENDA_OK:                   return "ok";
    case AGENDA_ERR_ARGUMENT:         return "invalid argument";
    case AGENDA_ERR_FORMAT:           return "bad format (expected e.g. MON 08:00-09:40)";
    case AGENDA_ERR_FULL:             return "schedule is full";
    case AGENDA_ERR_BUFFER_TOO_SMALL: return "buffer too small";
    case AGENDA_ERR_RANGE:            return "index out of range";
    case AGENDA_ERR_NO_MEMORY:        return "out of memory";
    }
    return "unknown error";
}

/* Reaproveita a validação de dias da API, sem repetir a tabela de nomes. */
static int day_from_name(const char *name)
{
    char text[TS_TEXT_MAX];
    ts_event ev;

    if (strlen(name) != 3) {
        return -1;
    }
    snprintf(text, sizeof text, "%s 00:00-24:00", name);
    return ts_parse(text, &ev) == AGENDA_OK ? ev.day : -1;
}

static void print_clock(int minutes)
{
    printf("%02d:%02d", minutes / 60, minutes % 60);
}

static void cmd_add(schedule *s, const char *text)
{
    ts_event ev;
    agenda_status rc = ts_parse(text, &ev);

    if (rc == AGENDA_OK) {
        rc = sch_add(s, &ev);
    }
    if (rc == AGENDA_OK) {
        printf("added %s\n", text);
    } else {
        printf("error: %s\n", status_text(rc));
    }
}

static void cmd_list(const schedule *s)
{
    size_t i;
    size_t n = sch_count(s);
    ts_event ev;
    char text[TS_TEXT_MAX];
    int minutes;

    if (n == 0) {
        printf("(no events)\n");
    }
    for (i = 0; i < n; i++) {
        if (sch_get(s, i, &ev) == AGENDA_OK && ts_format(&ev, text, sizeof text) == AGENDA_OK &&
            ts_duration(&ev, &minutes) == AGENDA_OK) {
            printf("%2zu  %s  (%d min)\n", i, text, minutes);
        }
    }
}

static void print_conflict(const schedule *s, const cf_pair *pair)
{
    ts_event first_event;
    ts_event second_event;
    char first_text[TS_TEXT_MAX];
    char second_text[TS_TEXT_MAX];
    int shared_minutes;

    if (sch_get(s, pair->first, &first_event) != AGENDA_OK ||
        sch_get(s, pair->second, &second_event) != AGENDA_OK ||
        ts_format(&first_event, first_text, sizeof first_text) != AGENDA_OK ||
        ts_format(&second_event, second_text, sizeof second_text) != AGENDA_OK ||
        ts_overlaps(&first_event, &second_event, &shared_minutes) != AGENDA_OK) {
        return;
    }
    printf("conflict: %s  x  %s  (%d min shared)\n",
           first_text, second_text, shared_minutes);
}

static void cmd_conflicts(const schedule *s)
{
    cf_pair pairs[MAX_PAIRS];
    size_t count = 0;
    size_t i;
    int has = 0;
    agenda_status rc = cf_has_conflict(s, &has);

    if (rc != AGENDA_OK) {
        printf("error: %s\n", status_text(rc));
        return;
    }
    if (!has) {
        printf("no conflicts\n");
        return;
    }
    rc = cf_find_all(s, pairs, MAX_PAIRS, &count);
    if (rc != AGENDA_OK && rc != AGENDA_ERR_BUFFER_TOO_SMALL) {
        printf("error: %s\n", status_text(rc));
        return;
    }
    for (i = 0; i < count && i < MAX_PAIRS; i++) {
        print_conflict(s, &pairs[i]);
    }
    if (count > MAX_PAIRS) {
        printf("... and %zu more\n", count - MAX_PAIRS);
    }
}

static void cmd_free(const schedule *s, const char *day_name)
{
    ts_event w[MAX_WINDOWS];
    size_t count = 0;
    size_t i;
    int day = day_from_name(day_name);
    agenda_status rc;

    if (day < 0) {
        printf("error: unknown day \"%s\" (use MON TUE WED THU FRI SAT SUN)\n", day_name);
        return;
    }
    rc = ft_find_free(s, day, w, MAX_WINDOWS, &count);
    if (rc != AGENDA_OK && rc != AGENDA_ERR_BUFFER_TOO_SMALL) {
        printf("error: %s\n", status_text(rc));
        return;
    }
    if (count == 0) {
        printf("%s: no free time\n", day_name);
    }
    for (i = 0; i < count && i < MAX_WINDOWS; i++) {
        printf("%s free ", day_name);
        print_clock(w[i].start);
        printf("-");
        print_clock(w[i].end);
        printf("  (%d min)\n", w[i].end - w[i].start);
    }
}

static void cmd_busy(const schedule *s, const char *day_name)
{
    int day = day_from_name(day_name);
    int busy = 0;
    agenda_status rc;

    if (day < 0) {
        printf("error: unknown day \"%s\" (use MON TUE WED THU FRI SAT SUN)\n", day_name);
        return;
    }
    rc = ft_total_busy(s, day, &busy);
    if (rc != AGENDA_OK) {
        printf("error: %s\n", status_text(rc));
        return;
    }
    printf("%s busy %d min (%d h %02d min)\n", day_name, busy, busy / 60, busy % 60);
}

static void print_help(void)
{
    printf("commands:\n"
           "  add DAY HH:MM-HH:MM   e.g. add MON 08:00-09:40\n"
           "  list                  list events in order\n"
           "  conflicts             list conflicting pairs\n"
           "  free DAY              free windows of a day\n"
           "  busy DAY              busy minutes of a day\n"
           "  help | quit\n");
}

/* Retorna zero para encerrar a leitura e um para continuar. */
static int run_command(schedule *s, char *line)
{
    char cmd[16];
    char arg[64];
    int fields;

    line[strcspn(line, "\r\n")] = '\0';
    fields = sscanf(line, "%15s %63[^\n]", cmd, arg);
    if (fields < 1) {
        return 1;
    }
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        return 0;
    } else if (strcmp(cmd, "help") == 0) {
        print_help();
    } else if (strcmp(cmd, "list") == 0) {
        cmd_list(s);
    } else if (strcmp(cmd, "conflicts") == 0) {
        cmd_conflicts(s);
    } else if (fields == 2 && strcmp(cmd, "add") == 0) {
        cmd_add(s, arg);
    } else if (fields == 2 && strcmp(cmd, "free") == 0) {
        cmd_free(s, arg);
    } else if (fields == 2 && strcmp(cmd, "busy") == 0) {
        cmd_busy(s, arg);
    } else {
        printf("unknown or incomplete command: \"%s\" (type help)\n", line);
    }
    return 1;
}

static void run_demo(schedule *s)
{
    static const char *const script[] = {
        "add MON 08:00-09:40",
        "add MON 09:20-11:00",
        "add MON 11:00-12:00",
        "add TUE 14:00-16:00",
        "add MON 25:00-26:00",
        "list",
        "conflicts",
        "free MON",
        "busy MON",
        "free WED"
    };
    size_t i;

    for (i = 0; i < sizeof script / sizeof script[0]; i++) {
        char line[80];

        printf("> %s\n", script[i]);
        snprintf(line, sizeof line, "%s", script[i]);
        run_command(s, line);
    }
}

int main(int argc, char **argv)
{
    schedule *s = NULL;
    agenda_status rc = sch_new(CAPACITY, &s);

    if (rc != AGENDA_OK) {
        fprintf(stderr, "cannot create schedule: %s\n", status_text(rc));
        return 1;
    }
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        run_demo(s);
    } else {
        char line[128];

        while (fgets(line, sizeof line, stdin) != NULL) {
            if (!run_command(s, line)) {
                break;
            }
        }
    }
    sch_free(s);
    return 0;
}
