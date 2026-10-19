#ifndef FREETIME_H
#define FREETIME_H

#include <stddef.h>
#include "agenda_error.h"
#include "timeslot.h"
#include "schedule.h"

/*
 * ft_find_free: lista as janelas livres de um dia, ordenadas por inicio.
 * Cada janela e escrita como ts_event com o dia pedido; eventos
 * sobrepostos ou que so encostam nao criam janela falsa.
 *   Dia:        0 <= Dia < TS_DAYS.
 *   OutPtr:     buffer do chamador com espaco para Capacidade janelas
 *               (pode ser nulo se Capacidade for 0).
 *   Cont:       recebe a quantidade de janelas escritas; se o buffer for
 *               pequeno demais, recebe a quantidade necessaria.
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se S ou Cont forem nulos, se OutPtr
 *          for nulo com Capacidade > 0 ou se Dia for invalido;
 *          AGENDA_ERR_BUFFER_TOO_SMALL se Capacidade for menor que a quantidade
 *          de janelas (as primeiras Capacidade janelas sao escritas).
 */
agenda_status ft_find_free(const schedule *S, int Dia, ts_event *OutPtr,
                           size_t Capacidade, size_t *Cont);

/*
 * ft_total_busy: escreve em *OutPtr o total de minutos ocupados no dia,
 * sem contar duas vezes o tempo sobreposto (resultado entre 0 e 1440).
 * Retorno: AGENDA_OK; AGENDA_ERR_ARGUMENT se S ou OutPtr forem nulos ou se
 *          Dia for invalido.
 */
agenda_status ft_total_busy(const schedule *S, int Dia, int *OutPtr);

#endif
