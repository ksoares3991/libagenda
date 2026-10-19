#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"

typedef struct Agenda schedule;

/*
 * sch_new: cria uma agenda vazia com capacidade fixa.
 *   Capacidade: numero maximo de eventos (maior que 0).
 *   OutPtr:     recebe a agenda criada; fica NULL se houver erro.
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se OutPtr for nulo ou Capacidade for 0;
 *          AGENDA_ERR_NO_MEMORY se faltar memoria.
 * A agenda deve ser liberada com sch_free.
 */
agenda_status sch_new(size_t Capacidade, schedule **OutPtr);

/*
 * sch_free: libera a agenda. Aceita ponteiro nulo.
 */
void sch_free(schedule *S);

/*
 * sch_add: acrescenta uma copia do evento ao fim da agenda.
 * Evento valido: 0 <= day < TS_DAYS e 0 <= start < end <= TS_DAY_MINUTES.
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se S ou Event forem nulos ou se o
 *          evento for invalido; AGENDA_ERR_FULL se a agenda ja estiver na capacidade.
 */
agenda_status sch_add(schedule *S, const ts_event *Event);

/*
 * sch_count: escreve em *OutPtr a quantidade de eventos.
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se S ou OutPtr forem nulos.
 */
agenda_status sch_count(const schedule *S, size_t *OutPtr);

/*
 * sch_get: copia o evento de indice Ev_indx (ordem de insercao) para *OutPtr.
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se S ou OutPtr forem nulos;
 *          AGENDA_ERR_RANGE se Ev_indx >= quantidade de eventos.
 */
agenda_status sch_get(const schedule *S, size_t Ev_indx, ts_event *OutPtr);

#endif
