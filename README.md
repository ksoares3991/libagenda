# libagenda

Biblioteca em C para uma **agenda semanal de estudantes**: guarda os eventos da semana, detecta **conflitos de horário** e calcula **horários livres**. Trabalho Prático 2 de MAC0216 (Técnicas de Programação).

A biblioteca tem 4 módulos e 13 funções públicas, é gerada como biblioteca **estática** (`libagenda.a`) e **dinâmica** (`libagenda.so`), tem testes automatizados e uma aplicação cliente em linha de comando.

## Pré-requisitos

- Linux
- GCC e Make
- Git
- Opcional: Valgrind (para conferir vazamentos de memória)

## Estrutura do projeto

```
libagenda/
├── include/            interface pública (os únicos arquivos que o cliente inclui)
│   ├── agenda_error.h    enum de erro único: agenda_status
│   ├── timeslot.h        evento: texto, duração, sobreposição
│   ├── schedule.h        agenda (tipo opaco), capacidade fixa
│   ├── conflicts.h       detecção de conflitos
│   └── freetime.h        janelas livres e tempo ocupado
├── src/                implementação dos módulos (timeslot.c, schedule.c, conflicts.c, freetime.c)
├── app/main.c          aplicação cliente (usa só a API pública)
├── tests/              check.h e um test_<módulo>.c por módulo, mais test_random.c
├── build/ lib/ bin/    gerados pela compilação (não vão para o Git)
├── docs/               ferramentas auxiliares (mutation_test.py)
├── Makefile
└── README.md
```

| Módulo | Responsabilidade | Funções públicas |
| --- | --- | --- |
| `timeslot` | representar e comparar um evento (dia e horários) | `ts_parse`, `ts_format`, `ts_duration`, `ts_overlaps` |
| `schedule` | guardar os eventos, em ordem, com capacidade fixa (tipo opaco) | `sch_new`, `sch_free`, `sch_add`, `sch_count`, `sch_get` |
| `conflicts` | detectar conflitos entre eventos | `cf_has_conflict`, `cf_find_all` |
| `freetime` | janelas livres e tempo ocupado de um dia | `ft_find_free`, `ft_total_busy` |

Dependências: `timeslot` é a base; `schedule` usa `timeslot`; `conflicts` e `freetime` usam `schedule` e `timeslot` e não dependem um do outro.

## Compilação

```
make            # gera lib/libagenda.a, lib/libagenda.so, bin/agenda_static e bin/agenda_shared
make clean      # apaga build/, lib/ e bin/
```

### Alvos do Makefile

| Alvo | O que faz |
| --- | --- |
| `make` / `make all` | compila as duas bibliotecas e os dois executáveis do app |
| `make run` | compila se necessário e executa a demonstração com o app **estático** |
| `make run-shared` | o mesmo com o app **dinâmico** |
| `make test` | compila e executa todos os testes; imprime `ALL TESTS PASSED` ou termina com erro |
| `make install` | instala em `PREFIX` (padrão `/usr/local`) |
| `make uninstall` | remove o que o `install` copiou |
| `make sanitize` | roda os testes com AddressSanitizer e UBSan (limpa a árvore antes e depois) |
| `make clean` | remove os artefatos gerados |
| `make help` | lista os alvos |

### Compilação incremental

O Makefile usa `-MMD -MP`: o GCC grava, ao lado de cada `.o`, um arquivo `.d` com os headers que o `.c` incluiu, e o Make lê esses arquivos. Assim:

- alterar `src/conflicts.c` recompila só `conflicts` (e refaz as bibliotecas e os executáveis);
- alterar `include/timeslot.h` recompila só os arquivos que incluem `timeslot.h`.

### Biblioteca estática e dinâmica

- Estática: os objetos de `src/` entram em `lib/libagenda.a` (com `ar`). `bin/agenda_static` copia o código da biblioteca para dentro do executável.
- Dinâmica: os objetos são compilados com `-fPIC` e unidos com `-shared` em `lib/libagenda.so`. `bin/agenda_shared` só registra a dependência; o carregador dinâmico resolve a biblioteca na execução. O executável usa `-Wl,-rpath,'$ORIGIN/../lib'`, então roda sem `LD_LIBRARY_PATH`, tanto na árvore do projeto quanto depois do `make install`. Para ver a dependência: `ldd bin/agenda_shared`.

## Execução

```
make run                 # demonstração fixa, app estático
make run-shared          # demonstração fixa, app dinâmico
./bin/agenda_static      # modo interativo: lê comandos da entrada padrão
```

Comandos do modo interativo (também aceitos por entrada redirecionada):

```
add DAY HH:MM-HH:MM    adiciona um evento, ex.: add MON 08:00-09:40
list                   lista os eventos em ordem
conflicts              lista os pares de eventos em conflito
free DAY               janelas livres do dia, ex.: free MON
busy DAY               minutos ocupados do dia
help | quit
```

Dias: `MON TUE WED THU FRI SAT SUN`. Horas: `00:00` a `24:00`.

## Testes

```
make test        # roda todos os testes
make sanitize    # idem, com AddressSanitizer e UBSan
python3 docs/mutation_test.py   # teste de mutação (alguns minutos); veja o relatório, item 11
```

Cada módulo tem seu `tests/test_<módulo>.c`, com casos normais, casos limite e entradas inválidas para **todas as funções públicas**. `tests/test_random.c` compara a biblioteca com um oráculo simples (um bitmap de minutos) em agendas aleatórias.

## Instalação

```
make install PREFIX=$HOME/.local
```

Instala `libagenda.a` e `libagenda.so` em `PREFIX/lib`, os headers em `PREFIX/include/agenda/` e o app dinâmico como `PREFIX/bin/agenda`. Para usar a biblioteca instalada em outro programa:

```
gcc programa.c -I$HOME/.local/include/agenda -L$HOME/.local/lib -lagenda -Wl,-rpath,$HOME/.local/lib
```

`make uninstall PREFIX=$HOME/.local` remove os arquivos.

## Decisões de API

| ID | Decisão |
| --- | --- |
| D1 | Um evento é um dia da semana mais um horário de início e um de fim. Só há conflito entre eventos do mesmo dia. |
| D2 | Os horários são guardados em minutos desde 00:00, de 0 a 1440. O texto usa `HH:MM`. |
| D3 | O intervalo é `[início, fim)`: inclui o início e exclui o fim. Eventos que só encostam não conflitam (08:00–09:40 e 09:40–10:00); 08:00–09:40 e 09:20–11:00 conflitam por 20 min. |
| D4 | Entrada inválida devolve erro, nunca um valor "corrigido". |
| D5 | Um único enum de erros (`agenda_status`) em `agenda_error.h`. As funções devolvem o código e entregam o resultado por ponteiro de saída. |
| D6 | Só `sch_new` e `sch_free` alocam e liberam memória. As demais funções escrevem em buffer do chamador. Resultados saem ordenados por dia e depois por início. |
| D7 | A capacidade da agenda é fixa e informada em `sch_new`. Agenda cheia é erro em `sch_add`. |
| D8 | `schedule` é um tipo opaco, então `sch_get` é necessária para ler os eventos. |

Detalhes decididos na implementação:

- A agenda mantém os eventos ordenados por (dia, início, fim); `sch_get(s, 0, ...)` é o primeiro evento da semana. `conflicts` e `freetime` usam essa garantia, documentada em `schedule.h`.
- `conflicts` usa a ordenação da agenda (varredura única). `freetime` não depende dela: marca em um vetor de 1440 posições, uma por minuto, os minutos ocupados do dia; as janelas livres são as sequências de minutos livres e o tempo ocupado é o total de minutos marcados.
- `ft_find_free` escreve cada janela como um `ts_event` (o dia pedido, início e fim), então uma janela pode ser impressa com `ts_format`.
- Formato do texto: `MON 08:00-09:40`. Texto fora do formato ou com horário fora de 00:00–24:00 é `AGENDA_ERR_FORMAT`; texto válido com fim ≤ início é `AGENDA_ERR_ARGUMENT`.
- Quando o buffer do chamador é pequeno (`cf_find_all`, `ft_find_free`), a função escreve os primeiros `cap` resultados, devolve `AGENDA_ERR_BUFFER_TOO_SMALL` e informa em `*count` quantos seriam necessários, para o chamador tentar de novo.

## Tratamento de erros

Toda função que pode falhar devolve `agenda_status` (`include/agenda_error.h`):

| Código | Significado |
| --- | --- |
| `AGENDA_OK` (0) | sucesso |
| `AGENDA_ERR_ARGUMENT` | ponteiro nulo, valor fora dos limites ou intervalo inconsistente |
| `AGENDA_ERR_FORMAT` | texto fora do formato esperado |
| `AGENDA_ERR_FULL` | agenda sem capacidade |
| `AGENDA_ERR_BUFFER_TOO_SMALL` | buffer do chamador pequeno demais |
| `AGENDA_ERR_RANGE` | índice fora da faixa |
| `AGENDA_ERR_NO_MEMORY` | falha de alocação |

Quais códigos cada função devolve está documentado no header do módulo, junto da função.

## Git

Cada módulo foi desenvolvido em uma branch `feature/<módulo>` com os testes primeiro (commit que falha contra o stub) e a implementação depois (commit que passa), integrada com `git merge --no-ff`. A versão final está na tag `v1.0`.

```
git log --graph --oneline --decorate
git show v1.0
```
