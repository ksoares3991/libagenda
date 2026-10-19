#ifndef FREETIME_H
#define FREETIME_H

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"
#include "schedule.h"

/*
 * Lista em buf as janelas livres de s no dia day (0 a 6), em ordem de início.
 * cap é a capacidade em eventos; buf pode ser NULL somente com cap zero.
 * count recebe o total necessário, mesmo quando só os primeiros cap cabem.
 * Dia vazio produz 00:00-24:00; dia inteiramente ocupado não produz janelas.
 * Retorna OK, ARGUMENT para dia ou ponteiro inválido, ou BUFFER_TOO_SMALL
 * se houver mais janelas que espaço disponível.
 */
agenda_status ft_find_free(const schedule *s, int day, ts_event *buf, size_t cap, size_t *count);

/*
 * Grava em busy os minutos ocupados de s no dia day (0 a 6).
 * Sobreposições contam uma vez. Retorna OK ou ARGUMENT para
 * s/busy nulos ou dia fora da faixa.
 */
agenda_status ft_total_busy(const schedule *s, int day, int *busy);

#endif
