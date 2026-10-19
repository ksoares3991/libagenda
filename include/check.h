#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>

static int Ver_total = 0;
static int Ver_falha = 0;

#define VER(cond) \
    do { \
        Ver_total++; \
        if (!(cond)) { \
            Ver_falha++; \
            printf("  FALHA %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define VER_EQ(a, b) \
    do { \
        long long va_ = (long long)(a), vb_ = (long long)(b); \
        Ver_total++; \
        if (va_ != vb_) { \
            Ver_falha++; \
            printf("  FALHA %s:%d: %s == %s (obtido %lld, esperado %lld)\n", \
                   __FILE__, __LINE__, #a, #b, va_, vb_); \
        } \
    } while (0)

#define VER_REPT(Nome) \
    do { \
        printf("%s: %d verificacoes, %d falhas -> %s\n", Nome, Ver_total, \
               Ver_falha, Ver_falha ? "FALHOU" : "OK"); \
        return Ver_falha ? 1 : 0; \
    } while (0)

#endif
