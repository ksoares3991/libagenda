#include <stdint.h>
#include <stdlib.h>
#include "schedule.h"

struct Agenda {
    ts_event *Itens;
    size_t Cont;
    size_t Fix_Cap;
};

static int Valid_Evt(const ts_event *Event)
{
    return Event->day >= 0 && Event->day < TS_DAYS
        && Event->start >= 0 && Event->start < Event->end
        && Event->end <= TS_DAY_MINUTES;
}

agenda_status sch_new(size_t Capacidade, schedule **OutPtr)
{
    schedule *Ag;

    if (OutPtr == NULL)
        return AGENDA_ERR_ARGUMENT;
    *OutPtr = NULL;
    if (Capacidade == 0)
        return AGENDA_ERR_ARGUMENT;
    if (Capacidade > SIZE_MAX / sizeof(ts_event))
        return AGENDA_ERR_NO_MEMORY;

    Ag = malloc(sizeof *Ag);
    if (Ag == NULL)
        return AGENDA_ERR_NO_MEMORY;
    Ag->Itens = malloc(Capacidade * sizeof(ts_event));
    if (Ag->Itens == NULL) {
        free(Ag);
        return AGENDA_ERR_NO_MEMORY;
    }
    Ag->Cont = 0;
    Ag->Fix_Cap = Capacidade;
    *OutPtr = Ag;
    return AGENDA_OK;
}

void sch_free(schedule *S)
{
    if (S == NULL)
        return;
    free(S->Itens);
    free(S);
}

agenda_status sch_add(schedule *S, const ts_event *Event)
{
    if (S == NULL || Event == NULL)
        return AGENDA_ERR_ARGUMENT;
    if (!Valid_Evt(Event))
        return AGENDA_ERR_ARGUMENT;
    if (S->Cont == S->Fix_Cap)
        return AGENDA_ERR_FULL;
    S->Itens[S->Cont++] = *Event;
    return AGENDA_OK;
}

agenda_status sch_count(const schedule *S, size_t *OutPtr)
{
    if (S == NULL || OutPtr == NULL)
        return AGENDA_ERR_ARGUMENT;
    *OutPtr = S->Cont;
    return AGENDA_OK;
}

agenda_status sch_get(const schedule *S, size_t Ev_indx, ts_event *OutPtr)
{
    if (S == NULL || OutPtr == NULL)
        return AGENDA_ERR_ARGUMENT;
    if (Ev_indx >= S->Cont)
        return AGENDA_ERR_RANGE;
    *OutPtr = S->Itens[Ev_indx];
    return AGENDA_OK;
}
