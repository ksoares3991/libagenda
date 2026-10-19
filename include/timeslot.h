#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <stddef.h>
#include "agenda_error.h"

#define TS_DAYS        7
#define TS_DAY_MINUTES 1440
#define TS_TEXT_MAX    16

/* Dia: 0 (segunda) a 6 (domingo). Horários em minutos, com 0 <= início < fim <= 1440. */
typedef struct {
    int day;
    int start;
    int end;
} ts_event;

/*
 * Lê text no formato "MON 08:00-09:40" e grava o evento em out.
 * Retorna OK, FORMAT para texto ou horário inválido, ou ARGUMENT para
 * ponteiro nulo ou fim <= início. Em caso de erro, não altera out.
 */
agenda_status ts_parse(const char *text, ts_event *out);

/*
 * Escreve ev em buf; cap é o tamanho disponível em bytes (16 bastam).
 * Retorna OK, ARGUMENT para ponteiro nulo ou evento inválido, ou
 * BUFFER_TOO_SMALL. Neste último caso, esvazia buf se cap > 0.
 */
agenda_status ts_format(const ts_event *ev, char *buf, size_t cap);

/*
 * Valida ev e grava sua duração em minutes.
 * Retorna OK ou ARGUMENT para evento inválido ou ponteiro nulo.
 */
agenda_status ts_duration(const ts_event *ev, int *minutes);

/*
 * Grava em minutes a sobreposição de a e b. Dias diferentes e intervalos
 * que apenas se encostam produzem zero. Retorna OK ou ARGUMENT
 * para evento inválido ou ponteiro nulo.
 */
agenda_status ts_overlaps(const ts_event *a, const ts_event *b, int *minutes);

#endif
