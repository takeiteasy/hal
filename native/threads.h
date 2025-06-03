/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef PAUL_THREADS_HEAD
#define PAUL_THREADS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define PAUL_ONLY_THREADS
#include "../paul.h"

typedef struct paul_cnd_t       paul_cnd_t;
typedef struct paul_thrd_t      paul_thrd_t;
typedef struct paul_tss_t       paul_tss_t;
typedef struct paul_mtx_t       paul_mtx_t;
typedef struct paul_once_flag   paul_once_flag;

typedef void (*tss_dtor_t)(void*);
typedef int (*thrd_start_t)(void*);

typedef struct paul_thrd_timeout paul_thrd_timeout;

enum {
    PAUL_MTX_PLAIN     = 0,
    PAUL_MTX_TRY       = 1,
    PAUL_MTX_TIMED     = 2,
    PAUL_MTX_RECURSIVE = 4
};

enum {
    PAUL_THRD_SUCCESS = 0, // succeeded
    PAUL_THRD_TIMEOUT,     // timeout
    PAUL_THRD_ERROR,       // failed
    PAUL_THRD_BUSY,        // resource busy
    PAUL_THRD_NOMEM        // out of memory
};

void paul_call_once(paul_once_flag *flag, void (*func)(void));

int paul_cnd_broadcast(paul_cnd_t *cond);
void paul_cnd_destroy(paul_cnd_t *cond);
int paul_cnd_init(paul_cnd_t *cond);
int paul_cnd_signal(paul_cnd_t *cond);
int paul_cnd_timedwait(paul_cnd_t *cond, paul_mtx_t *mtx, const paul_thrd_timeout *xt);
int paul_cnd_wait(paul_cnd_t *cond, paul_mtx_t *mtx);

void paul_mtx_destroy(paul_mtx_t *mtx);
int paul_mtx_init(paul_mtx_t *mtx, int type);
int paul_mtx_lock(paul_mtx_t *mtx);
int paul_mtx_timedlock(paul_mtx_t *mtx, const paul_thrd_timeout *xt);
int paul_mtx_trylock(paul_mtx_t *mtx);
int paul_mtx_unlock(paul_mtx_t *mtx);

int paul_thrd_create(paul_thrd_t *thr, thrd_start_t func, void *arg);
paul_thrd_t paul_thrd_current(void);
int paul_thrd_detach(paul_thrd_t thr);
int paul_thrd_equal(paul_thrd_t thr0, paul_thrd_t thr1);
void paul_thrd_exit(int res);
int paul_thrd_join(paul_thrd_t thr, int *res);
void paul_thrd_sleep(const paul_thrd_timeout *xt);
void paul_thrd_yield(void);

int paul_tss_create(paul_tss_t *key, tss_dtor_t dtor);
void paul_tss_delete(paul_tss_t key);
void* paul_tss_get(paul_tss_t key);
int paul_tss_set(paul_tss_t key, void *val);

int paul_timeout(paul_thrd_timeout *xt, int base);

#ifdef __cplusplus
}
#endif
#endif // PAUL_THREADS_HEAD
