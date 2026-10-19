#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"

/* Os campos da agenda ficam em schedule.c. O cliente usa as funções abaixo. */
typedef struct schedule schedule;

/*
 * Cria em out uma agenda para até capacity eventos, com capacidade fixa.
 * Retorna OK, ARGUMENT para out nulo ou capacity zero, ou NO_MEMORY
 * se não puder alocar. Deixa *out nulo em caso de erro, quando out existe.
 */
agenda_status sch_new(size_t capacity, schedule **out);

/* Libera s e seus eventos. Aceita NULL. */
void sch_free(schedule *s);

/*
 * Copia ev para s, em ordem de dia, início e fim. Aceita eventos iguais.
 * Retorna OK, ARGUMENT para ponteiro nulo ou evento inválido, ou FULL
 * para agenda cheia. Valida o evento antes de conferir a capacidade.
 * Não altera a agenda em caso de erro.
 */
agenda_status sch_add(schedule *s, const ts_event *ev);

/* Retorna a quantidade de eventos em s, ou zero se s for NULL. */
size_t sch_count(const schedule *s);

/*
 * Copia para out o evento de índice index, começando em zero.
 * Retorna OK, ARGUMENT para ponteiro nulo ou RANGE para índice inexistente.
 */
agenda_status sch_get(const schedule *s, size_t index, ts_event *out);

#endif
