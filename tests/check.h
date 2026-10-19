#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include <string.h>

/* Cada programa de teste mantém seus próprios contadores. */
static int check_total = 0;
static int check_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        check_total++;                                                       \
        if (!(cond)) {                                                       \
            check_failed++;                                                  \
            fprintf(stderr, "  FAIL %s:%d: CHECK(%s)\n",                     \
                    __FILE__, __LINE__, #cond);                              \
        }                                                                    \
    } while (0)

#define CHECK_EQ(actual, expected)                                           \
    do {                                                                     \
        long long check_a_ = (long long)(actual);                            \
        long long check_e_ = (long long)(expected);                          \
        check_total++;                                                       \
        if (check_a_ != check_e_) {                                          \
            check_failed++;                                                  \
            fprintf(stderr, "  FAIL %s:%d: %s is %lld, expected %lld\n",     \
                    __FILE__, __LINE__, #actual, check_a_, check_e_);        \
        }                                                                    \
    } while (0)

#define CHECK_STREQ(actual, expected)                                        \
    do {                                                                     \
        const char *check_sa_ = (actual);                                    \
        const char *check_se_ = (expected);                                  \
        check_total++;                                                       \
        if (strcmp(check_sa_, check_se_) != 0) {                             \
            check_failed++;                                                  \
            fprintf(stderr, "  FAIL %s:%d: %s is \"%s\", expected \"%s\"\n", \
                    __FILE__, __LINE__, #actual, check_sa_, check_se_);      \
        }                                                                    \
    } while (0)

static int check_summary(const char *suite)
{
    if (check_failed == 0) {
        printf("[PASS] %-12s %d checks\n", suite, check_total);
        return 0;
    }
    printf("[FAIL] %-12s %d of %d checks failed\n", suite, check_failed, check_total);
    return 1;
}

#endif
