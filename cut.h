#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#ifndef CUT_H_
#define CUT_H_

#ifndef CUT_CAPACITY_STEP
#define CUT_CAPACITY_STEP 32
#endif CUT_CAPACITY_STEP

#if CUT_CAPACITY_STEP < 1
#error "CUT_CAPACITY_STEP must be greater than one"
#endif

typedef struct {
    void   **items;
    size_t size;
    size_t capacity;
} Cut_List;

/* TODO */
Cut_List *cut_list_new();

/* TODO */
Cut_List *cut_list_from_arr(void **arr, size_t size);

/* TODO */
bool cut_list_free_struct(Cut_List *ls);

/* TODO */
bool cut_list_free(Cut_List *ls);

/* TODO */
bool cut_list_push(Cut_List *ls, void *s);

/* TODO */
int cut_list_comp(const void *a, const void *b);

/* TODO */
void cut_list_sort(Cut_List *ls);

/* TODO */
bool cut_list_eq(Cut_List *ls1, Cut_List *ls2);

/* TODO */
char *cut_str_clone(const char *s);

#endif // CUT_H_

#ifdef CUT_IMPLEMENTATION

static char *current_test = NULL;
static Cut_List *failed_tests = NULL;
static bool skipped = false;
static bool in_assert_block = false;
static bool in_assert_manual = false;

#ifdef CUT_DEBUG
static void _cut_err(const char *file, int line, const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[%s:%d] %s: ", file, line, func);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}
#define CUT_THROW(ret, fmt, ...) ({ _cut_err(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__); return ret; })
#else
#define CUT_THROW(ret, fmt, ...) ({ return ret; })
#endif // CUT_DEBUG

#define CUT_ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define CUT_LIST_FROM(arr) \
    cut_list_from_arr((void**)arr, sizeof(arr)/sizeof(arr[0]))

Cut_List *cut_list_new() {
    Cut_List *ls = malloc(sizeof(Cut_List));
    if (!ls) CUT_THROW(NULL, "malloc error");

    ls->size = 0;
    ls->capacity = CUT_CAPACITY_STEP;

    ls->items = malloc(ls->capacity * sizeof(void *));
    if (!ls->items) CUT_THROW(NULL, "malloc error for items");

    return ls;
}

Cut_List *cut_list_from_arr(void **arr, size_t size) {
    if (!arr) CUT_THROW(NULL, "arr evaluates to false");

    Cut_List *ls = cut_list_new();
    if (!ls) CUT_THROW(NULL, "vec_new failed");

    for (size_t i = 0; i < size; i++)
        if (!cut_list_push(ls, arr[i])) CUT_THROW(NULL, "cut_list_push failed");

    return ls;
}

bool cut_list_free_struct(Cut_List *ls) {
    if (!ls) CUT_THROW(false, "ls evaluates to false");

    if (ls->items) free(ls->items);
    free(ls);
    ls = NULL;

    return true;
}

bool cut_list_free(Cut_List *ls) {
    if (!ls) CUT_THROW(false, "ls evaluates to false");

    for (size_t i = 0; i < ls->size; i++)
        if (ls->items[i]) free(ls->items[i]);

    return cut_list_free_struct(ls);
}

bool cut_list_push(Cut_List *ls, void *s) {
    if (ls->size == ls->capacity) {
        ls->capacity += CUT_CAPACITY_STEP;

        void *tmp = realloc(ls->items, ls->capacity * sizeof(void *));
        if (!tmp) CUT_THROW(false, "realloc error");

        ls->items = tmp;
    }

    ls->items[ls->size++] = cut_str_clone((char*)s);

    return true;
}

int cut_list_comp(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;

    while (*s1 && *s2 && *s1 == *s2) s1++, s2++;

    int diff = (unsigned char)*s1 - (unsigned char)*s2;

    return diff;
}

void cut_list_sort(Cut_List *ls) {
    qsort(ls->items, ls->size, sizeof(char *), cut_list_comp);
}

bool cut_list_eq(Cut_List *ls1, Cut_List *ls2) {
    if (!ls1 | !ls2) return false;
    if (ls1->size != ls2->size) return false;

    for (size_t i = 0; i < ls1->size; i++) {
        char *s1 = ls1->items[i];
        char *s2 = ls2->items[i];

        while (*s1 && *s2) if (*s1++ != *s2++) return false;
    }

    return true;
}

char *cut_str_clone(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy) CUT_THROW(NULL, "malloc error");

    for (size_t i = 0; i < len; i++) copy[i] = s[i];

    return copy;
}

void TEST(const char *name) {
    current_test = cut_str_clone(name);
    skipped = false;
}

void _test_ok() {
    if (!skipped) printf("[OK ] %s\n", current_test);
    free(current_test);
}

void _test_err(const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    fprintf(stderr, "[ERR] %s: ", current_test);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);

    cut_list_push(failed_tests, current_test);
}

void _test_skip() {
    fprintf(stderr, "[SKP] %s: test skipped\n", current_test);
    skipped = true;
}

bool _assert(bool b, ...) {
    if (!b) {
        va_list args;
        va_start(args, b);

        char *msg = va_arg(args, char *);
        if (!msg) msg = cut_str_clone("assertion failed");
        _test_err(msg, args);

        va_end(args);
    }

    return b;
}

bool _assert_block(bool *results, size_t total) {
    for (size_t i = 0; i < total; i++)
        if (!results[i]) {
            _test_err("assertion %zu", i);
            return false;
        }

    return true;
}

bool _assert_arr_str(char **a1, size_t s1, char **a2, size_t s2) {
    if (s1 != s2) {
        _test_err("array sizes are different");
        _test_err("s1 = %zu, s2 = %zu", s1, s2);
        return false;
    }

    for (size_t i = 0; i < s1; i++) {
        if (strcmp(a1[i], a2[i]) != 0) {
            _test_err("elements at index %zu are different", i);
            _test_err("a1[%zu] = %s, a2[%zu] = %s", i, a1[i], i, a2[i]);
            return false;
        }
    }

    return true;
}

bool _assert_arr_int(int a1[], size_t s1, int a2[], size_t s2) {
    if (s1 != s2) {
        _test_err("array sizes are different");
        _test_err("s1 = %zu, s2 = %zu",s1, s2);
        return false;
    }

    for (size_t i = 0; i < s1; i++) {
        if (a1[i] != a2[i]) {
            _test_err("elements at index %zu are different", i);
            _test_err("a1[%zu] = %i, a2[%zu] = %i", i, a1[i], i, a2[i]);
            return false;
        }
    }

    return true;
}

int _test_suite(bool (**tests)(), size_t total) {
    failed_tests = cut_list_new();

    printf("running tests (%zu total)\n", total);
    printf("--------------------------------\n");

    for (size_t i = 0; i < total; i++) {
        if (!tests[i]()) return 1;
        _test_ok();
    }

    printf("--------------------------------\n");
    if (!failed_tests->size) {
        printf("all tests passed\n");
    } else {
        printf("the following tests failed\n");

        for (size_t i = 0; i < failed_tests->size; i++)
            printf("%zu %s\n", i, failed_tests->items[i]);
    }

    return 0;
}

#define ASSERT_ZERO(n, ...) ({                                  \
    bool ok = _assert(n == 0, ##__VA_ARGS__);                   \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_TRUE(b, ...) ({                                  \
    bool ok = _assert(b, ##__VA_ARGS__);                        \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_FALSE(b, ...) ({                                 \
    bool ok = _assert(!b, ##__VA_ARGS__);                       \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_NULL(cond, ...) ({                               \
    bool ok = _assert((cond == NULL), ##__VA_ARGS__);           \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_NOT_NULL(cond, ...) ({                           \
    bool ok = _assert((cond != NULL), ##__VA_ARGS__);           \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_STR(s1, s2, ...) ({                              \
    bool ok = _assert(strcmp(s1, s2) == 0, ##__VA_ARGS__);      \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_ARR_STR(a1, s1, a2, s2, ...) ({                  \
    bool ok = _assert_arr_str(a1, s1, a2, s2);                  \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_ARR_INT(a1, s1, a2, s2, ...) ({                  \
    bool ok = _assert_arr_int(a1, s1, a2, s2);                  \
    if (!in_assert_block) return ok;                            \
    ok;                                                         \
})

#define ASSERT_BLOCK(...) ({                                    \
    in_assert_block = true;                                     \
    bool results[] = { __VA_ARGS__ };                           \
    bool ok = _assert_block(results, CUT_ARR_LEN(results));     \
    in_assert_block = false;                                    \
    if (!in_assert_manual) return ok;                           \
    ok;                                                         \
})

#define ASSERT_MANUAL(...) ({                                   \
    in_assert_manual = true;                                    \
    bool _result = ASSERT_BLOCK(__VA_ARGS__);                   \
    in_assert_manual = false;                                   \
    _result;                                                    \
})

#define SKIP() {                                                \
    _test_skip();                                               \
    return true;                                                \
}

#define FAIL() {                                                \
    return false;                                               \
}

#define FREE(var) ({                                            \
    free(var);                                                  \
    if (!in_assert_block) return true;                          \
    true;                                                       \
})

#define FREE_ARRAY(arr, len) ({                                 \
    for (size_t i = 0; i < len; i++) free(arr[i]);              \
    free(arr);                                                  \
    true;                                                       \
})

#define TEST_SUITE(...) ({                                      \
    bool (*tests[])(void) = { __VA_ARGS__ };                    \
    return _test_suite(tests, CUT_ARR_LEN(tests));              \
})

#endif // CUT_IMPLEMENTATION
