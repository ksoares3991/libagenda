#include <string.h>
#include "freetime.h"

static agenda_status Set_ocupado(const schedule *S, int Dia, unsigned char *Ocup)
{
    size_t Qnt_Evnt, i;
    agenda_status Erro;
    ts_event Rd_Evnt;

    memset(Ocup, 0, TS_DAY_MINUTES);
    Erro = sch_count(S, &Qnt_Evnt);
    if (Erro != AGENDA_OK)
        return Erro;
    for (i = 0; i < Qnt_Evnt; i++) {
        Erro = sch_get(S, i, &Rd_Evnt);
        if (Erro != AGENDA_OK)
            return Erro;
        if (Rd_Evnt.day == Dia)
            memset(Ocup + Rd_Evnt.start, 1, (size_t)(Rd_Evnt.end - Rd_Evnt.start));
    }
    return AGENDA_OK;
}

agenda_status ft_total_busy(const schedule *S, int Dia, int *OutPtr)
{
    unsigned char Ocup[TS_DAY_MINUTES];
    agenda_status Erro;
    int m, Total = 0;

    if (S == NULL || OutPtr == NULL)
        return AGENDA_ERR_ARGUMENT;
    if (Dia < 0 || Dia >= TS_DAYS)
        return AGENDA_ERR_ARGUMENT;
    Erro = Set_ocupado(S, Dia, Ocup);
    if (Erro != AGENDA_OK)
        return Erro;
    for (m = 0; m < TS_DAY_MINUTES; m++)
        Total += Ocup[m];
    *OutPtr = Total;
    return AGENDA_OK;
}

agenda_status ft_find_free(const schedule *S, int Dia, ts_event *OutPtr,
                           size_t Capacidade, size_t *Cont)
{
    unsigned char Ocup[TS_DAY_MINUTES];
    agenda_status Erro;
    size_t Total = 0;
    int m = 0, Ini;

    if (S == NULL || Cont == NULL || (OutPtr == NULL && Capacidade > 0))
        return AGENDA_ERR_ARGUMENT;
    if (Dia < 0 || Dia >= TS_DAYS)
        return AGENDA_ERR_ARGUMENT;
    Erro = Set_ocupado(S, Dia, Ocup);
    if (Erro != AGENDA_OK)
        return Erro;

    while (m < TS_DAY_MINUTES) {
        if (Ocup[m]) {
            m++;
            continue;
        }
        Ini = m;
        while (m < TS_DAY_MINUTES && !Ocup[m])
            m++;
        if (Total < Capacidade) {
            OutPtr[Total].day = Dia;
            OutPtr[Total].start = Ini;
            OutPtr[Total].end = m;
        }
        Total++;
    }
    *Cont = Total;
    return Total > Capacidade ? AGENDA_ERR_BUFFER_TOO_SMALL : AGENDA_OK;
}
