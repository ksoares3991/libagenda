#include "check.h"
#include "freetime.h"

static schedule *Make(size_t Cap)
{
    schedule *Ag = NULL;
    sch_new(Cap, &Ag);
    return Ag;
}

static void Add(schedule *Ag, int day, int start, int end)
{
    ts_event ev;
    ev.day = day;
    ev.start = start;
    ev.end = end;
    sch_add(Ag, &ev);
}

static void Tst_dia_vazio(void)
{
    schedule *Ag = Make(4);
    ts_event win[4];
    size_t Qnt_win;
    int busy;

    VER_EQ(ft_total_busy(Ag, 0, &busy), AGENDA_OK);
    VER_EQ(busy, 0);
    VER_EQ(ft_find_free(Ag, 0, win, 4, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 1);
    VER(win[0].day == 0 && win[0].start == 0 && win[0].end == 1440);
    sch_free(Ag);
}

static void Tst_dia_cheio(void)
{
    schedule *Ag = Make(2);
    ts_event win[2];
    size_t Qnt_win = 7;
    int busy;

    Add(Ag, 2, 0, 1440);
    VER_EQ(ft_total_busy(Ag, 2, &busy), AGENDA_OK);
    VER_EQ(busy, 1440);
    VER_EQ(ft_find_free(Ag, 2, win, 2, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 0);
    sch_free(Ag);
}

static void Tst_janela_entre_evnt(void)
{
    schedule *Ag = Make(4);
    ts_event win[4];
    size_t Qnt_win;
    int busy;

    Add(Ag, 1, 720, 780);
    Add(Ag, 1, 480, 600);
    VER_EQ(ft_total_busy(Ag, 1, &busy), AGENDA_OK);
    VER_EQ(busy, 180);
    VER_EQ(ft_find_free(Ag, 1, win, 4, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 3);
    VER(win[0].day == 1 && win[0].start == 0 && win[0].end == 480);
    VER(win[1].start == 600 && win[1].end == 720);
    VER(win[2].start == 780 && win[2].end == 1440);
    sch_free(Ag);
}

static void Tst_overlap_cont_uniq(void)
{
    schedule *Ag = Make(4);
    ts_event win[4];
    size_t Qnt_win;
    int busy;

    Add(Ag, 0, 480, 600);
    Add(Ag, 0, 540, 660);
    Add(Ag, 0, 500, 520);
    VER_EQ(ft_total_busy(Ag, 0, &busy), AGENDA_OK);
    VER_EQ(busy, 180);
    VER_EQ(ft_find_free(Ag, 0, win, 4, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 2);
    VER(win[0].start == 0 && win[0].end == 480);
    VER(win[1].start == 660 && win[1].end == 1440);
    sch_free(Ag);
}

static void Tst_evnt_paralelos(void)
{
    schedule *Ag = Make(4);
    ts_event win[4];
    size_t Qnt_win;
    int busy;

    Add(Ag, 0, 480, 540);
    Add(Ag, 0, 540, 600);
    VER_EQ(ft_total_busy(Ag, 0, &busy), AGENDA_OK);
    VER_EQ(busy, 120);
    VER_EQ(ft_find_free(Ag, 0, win, 4, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 2);
    VER(win[0].end == 480 && win[1].start == 600);
    sch_free(Ag);
}

static void Tst_evnt_at_edge(void)
{
    schedule *Ag = Make(2);
    ts_event win[2];
    size_t Qnt_win;

    Add(Ag, 0, 0, 60);
    Add(Ag, 0, 1380, 1440);
    VER_EQ(ft_find_free(Ag, 0, win, 2, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 1);
    VER(win[0].start == 60 && win[0].end == 1380);
    sch_free(Ag);
}

static void Tst_ignore_outros(void)
{
    schedule *Ag = Make(3);
    ts_event win[2];
    size_t Qnt_win;
    int busy;

    Add(Ag, 3, 480, 600);
    VER_EQ(ft_total_busy(Ag, 4, &busy), AGENDA_OK);
    VER_EQ(busy, 0);
    VER_EQ(ft_find_free(Ag, 4, win, 2, &Qnt_win), AGENDA_OK);
    VER_EQ(Qnt_win, 1);
    VER(win[0].start == 0 && win[0].end == 1440);
    sch_free(Ag);
}

static void Tst_Ocup_less_than_Dia(void)
{
    schedule *Ag = Make(3);
    int busy;

    Add(Ag, 0, 0, 1440);
    Add(Ag, 0, 0, 1440);
    Add(Ag, 0, 100, 1000);
    VER_EQ(ft_total_busy(Ag, 0, &busy), AGENDA_OK);
    VER_EQ(busy, 1440);
    sch_free(Ag);
}

static void Tst_buffer_pequeno(void)
{
    schedule *Ag = Make(3);
    ts_event win[1];
    size_t Qnt_win = 0;

    Add(Ag, 0, 100, 200);
    Add(Ag, 0, 300, 400);
    VER_EQ(ft_find_free(Ag, 0, win, 1, &Qnt_win), AGENDA_ERR_BUFFER_TOO_SMALL);
    VER_EQ(Qnt_win, 3);
    VER(win[0].start == 0 && win[0].end == 100);

    Qnt_win = 0;
    VER_EQ(ft_find_free(Ag, 0, NULL, 0, &Qnt_win), AGENDA_ERR_BUFFER_TOO_SMALL);
    VER_EQ(Qnt_win, 3);
    sch_free(Ag);
}

static void test_invalid_arguments(void)
{
    schedule *Ag = Make(1);
    ts_event win[1];
    size_t Qnt_win;
    int busy;

    VER_EQ(ft_total_busy(Ag, -1, &busy), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_total_busy(Ag, TS_DAYS, &busy), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_find_free(Ag, -1, win, 1, &Qnt_win), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_find_free(Ag, TS_DAYS, win, 1, &Qnt_win), AGENDA_ERR_ARGUMENT);

    VER_EQ(ft_total_busy(NULL, 0, &busy), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_total_busy(Ag, 0, NULL), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_find_free(NULL, 0, win, 1, &Qnt_win), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_find_free(Ag, 0, win, 1, NULL), AGENDA_ERR_ARGUMENT);
    VER_EQ(ft_find_free(Ag, 0, NULL, 1, &Qnt_win), AGENDA_ERR_ARGUMENT);
    sch_free(Ag);
}

int main(void)
{
    Tst_dia_vazio();
    Tst_dia_cheio();
    Tst_janela_entre_evnt();
    Tst_overlap_cont_uniq();
    Tst_evnt_paralelos();
    Tst_evnt_at_edge();
    Tst_ignore_outros();
    Tst_Ocup_less_than_Dia();
    Tst_buffer_pequeno();
    test_invalid_arguments();
    VER_REPT("freetime");
}
