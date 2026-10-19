#ifndef CONFLICTS_H
#define CONFLICTS_H

#include <stddef.h>
#include "agenda_error.h"
#include "schedule.h"

/* Índices de dois eventos em conflito na agenda ordenada; first < second. */
typedef struct {
    size_t first;
    size_t second;
} cf_pair;

/*
 * Grava 1 em has se s tiver algum conflito; caso contrário, grava 0.
 * Retorna OK ou ARGUMENT se s ou has for NULL.
 */
agenda_status cf_has_conflict(const schedule *s, int *has);

/*
 * Lista os pares de s em buf, ordenados por first e depois second.
 * cap é a capacidade em pares; buf pode ser NULL somente se cap for zero.
 * count recebe o total necessário, mesmo quando só os primeiros cap cabem.
 * Retorna OK, ARGUMENT para s/count nulos ou buf nulo com cap > 0,
 * ou BUFFER_TOO_SMALL se não couberem todos os pares.
 */
agenda_status cf_find_all(const schedule *s, cf_pair *buf, size_t cap, size_t *count);

#endif
