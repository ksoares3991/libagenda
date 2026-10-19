#include "check.h"
#include "schedule.h"

static void Tst_novo_e_livre(void)
{
    schedule *s = NULL;
    size_t n = 99;

    VER_EQ(sch_new(0, &s), AGENDA_ERR_ARGUMENT);
    VER(s == NULL);
    VER_EQ(sch_new(3, NULL), AGENDA_ERR_ARGUMENT);

    VER_EQ(sch_new(3, &s), AGENDA_OK);
    VER(s != NULL);
    VER_EQ(sch_count(s, &n), AGENDA_OK);
    VER_EQ(n, 0);
    sch_free(s);
    sch_free(NULL);
}

static void Tst_add_till_cheio(void)
{
    schedule *s;
    ts_event Evnt = {0, 480, 540};
    size_t n;

    VER_EQ(sch_new(2, &s), AGENDA_OK);
    VER_EQ(sch_add(s, &Evnt), AGENDA_OK);
    VER_EQ(sch_add(s, &Evnt), AGENDA_OK);
    VER_EQ(sch_add(s, &Evnt), AGENDA_ERR_FULL);
    VER_EQ(sch_count(s, &n), AGENDA_OK);
    VER_EQ(n, 2);
    sch_free(s);
}

static void Tst_add_nonvalid(void)
{
    schedule *s;
    ts_event Ok = {2, 0, 1440};
    ts_event Igual = {0, 600, 600};
    ts_event Invertido = {0, 700, 600};
    ts_event Post_final = {0, 600, 1441};
    ts_event Ini_negativo = {0, -1, 60};
    ts_event Dia_ExtrPequeno = {-1, 0, 60};
    ts_event Dia_MtoGrande = {TS_DAYS, 0, 60};
    size_t n;

    VER_EQ(sch_new(5, &s), AGENDA_OK);
    VER_EQ(sch_add(s, &Igual), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, &Invertido), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, &Post_final), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, &Ini_negativo), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, &Dia_ExtrPequeno), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, &Dia_MtoGrande), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(s, NULL), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_add(NULL, &Ok), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_count(s, &n), AGENDA_OK);
    VER_EQ(n, 0);
    VER_EQ(sch_add(s, &Ok), AGENDA_OK);
    sch_free(s);
}

static void Tst_count_nulo(void)
{
    schedule *s;
    size_t n;

    VER_EQ(sch_new(1, &s), AGENDA_OK);
    VER_EQ(sch_count(NULL, &n), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_count(s, NULL), AGENDA_ERR_ARGUMENT);
    sch_free(s);
}

static void Tst_gt(void)
{
    schedule *s;
    ts_event a = {1, 480, 600};
    ts_event b = {3, 0, 1440};
    ts_event out;

    VER_EQ(sch_new(4, &s), AGENDA_OK);
    VER_EQ(sch_get(s, 0, &out), AGENDA_ERR_RANGE);
    VER_EQ(sch_add(s, &a), AGENDA_OK);
    VER_EQ(sch_add(s, &b), AGENDA_OK);

    VER_EQ(sch_get(s, 0, &out), AGENDA_OK);
    VER(out.day == 1 && out.start == 480 && out.end == 600);
    VER_EQ(sch_get(s, 1, &out), AGENDA_OK);
    VER(out.day == 3 && out.start == 0 && out.end == 1440);

    VER_EQ(sch_get(s, 2, &out), AGENDA_ERR_RANGE);
    VER_EQ(sch_get(s, 0, NULL), AGENDA_ERR_ARGUMENT);
    VER_EQ(sch_get(NULL, 0, &out), AGENDA_ERR_ARGUMENT);
    sch_free(s);
}

static void Tst_evnt_copiado(void)
{
    schedule *s;
    ts_event Evnt = {0, 60, 120};
    ts_event out;

    VER_EQ(sch_new(1, &s), AGENDA_OK);
    VER_EQ(sch_add(s, &Evnt), AGENDA_OK);
    Evnt.start = 0;
    Evnt.end = 1;
    VER_EQ(sch_get(s, 0, &out), AGENDA_OK);
    VER(out.start == 60 && out.end == 120);
    sch_free(s);
}

int main(void)
{
    Tst_novo_e_livre();
    Tst_add_till_cheio();
    Tst_add_nonvalid();
    Tst_count_nulo();
    Tst_gt();
    Tst_evnt_copiado();
    VER_REPT("schedule");
}
