/* https://github.com/takeiteasy/hal

hal Copyright (C) 2025 George Watson

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

#ifndef HAL_THREADS_HEAD
#define HAL_THREADS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_THREADS
#include "../hal.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>

#if defined(EMULATED_THREADS_USE_NATIVE_CALL_ONCE) && (_WIN32_WINNT < 0x0600)
#error EMULATED_THREADS_USE_NATIVE_CALL_ONCE requires _WIN32_WINNT>=0x0600
#endif

#if defined(EMULATED_THREADS_USE_NATIVE_CV) && (_WIN32_WINNT < 0x0600)
#error EMULATED_THREADS_USE_NATIVE_CV requires _WIN32_WINNT>=0x0600
#endif

#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
#define ONCE_FLAG_INIT INIT_ONCE_STATIC_INIT
#else
#define ONCE_FLAG_INIT {0}
#endif
#define TSS_DTOR_ITERATIONS 1

#if _WIN32_WINNT >= 0x0600
// Prefer native WindowsAPI on newer environment.
#define EMULATED_THREADS_USE_NATIVE_CALL_ONCE
#define EMULATED_THREADS_USE_NATIVE_CV
#endif
#define EMULATED_THREADS_TSS_DTOR_SLOTNUM 64  // see TLS_MINIMUM_AVAILABLE

typedef struct hal_cnd_t {
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    CONDITION_VARIABLE condvar;
#else
    int blocked;
    int gone;
    int to_unblock;
    HANDLE sem_queue;
    HANDLE sem_gate;
    CRITICAL_SECTION monitor;
#endif
} hal_cnd_t;

typdef struct hal_thrd_t {
    HANDLE hndl;
} hal_thrd_t;

typedef struct hal_tss_t {
    DWORD key;
} hal_tss_t;

typedef struct hal_mtx_t {
    CRITICAL_SECTION cs;
} hal_mtx_t;

typedef struct hal_once_flag {
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
    INIT_ONCE status;
#else
    volatile LONG status;
#endif
} hal_once_flag;
#else
#include <pthread.h>

typedef struct hal_cnd_t {
    pthread_cond_t cnd;
} hal_cnd_t;

typedef struct hal_thrd_t {
    pthread_t thrd;
} hal_thrd_t;

typedef struct hal_tss_t {
    pthread_key_t key;
} hal_tss_t;

typedef struct hal_mtx_t {
    pthread_mutex_t mtx;
} hal_mtx_t;

typedef struct hal_once_flag {
    pthread_once_t flag;
} hal_once_flag;
#endif

#include <time.h>

typedef struct hal_thrd_timeout {
    time_t sec;
    long nsec;
} hal_thrd_timeout;

enum {
    HAL_MTX_PLAIN     = 0,
    HAL_MTX_TRY       = 1,
    HAL_MTX_TIMED     = 2,
    HAL_MTX_RECURSIVE = 4
};

enum {
    HAL_THRD_SUCCESS = 0, // succeeded
    HAL_THRD_TIMEOUT,     // timeout
    HAL_THRD_ERROR,       // failed
    HAL_THRD_BUSY,        // resource busy
    HAL_THRD_NOMEM        // out of memory
};

typedef void (*tss_dtor_t)(void*);
typedef int (*thrd_start_t)(void*);

bool hal_threads_available(void);

void hal_call_once(hal_once_flag *flag, void (*func)(void));

int hal_cnd_broadcast(hal_cnd_t *cond);
void hal_cnd_destroy(hal_cnd_t *cond);
int hal_cnd_init(hal_cnd_t *cond);
int hal_cnd_signal(hal_cnd_t *cond);
int hal_cnd_timedwait(hal_cnd_t *cond, hal_mtx_t *mtx, const hal_thrd_timeout *xt);
int hal_cnd_wait(hal_cnd_t *cond, hal_mtx_t *mtx);

void hal_mtx_destroy(hal_mtx_t *mtx);
int hal_mtx_init(hal_mtx_t *mtx, int type);
int hal_mtx_lock(hal_mtx_t *mtx);
int hal_mtx_timedlock(hal_mtx_t *mtx, const hal_thrd_timeout *xt);
int hal_mtx_trylock(hal_mtx_t *mtx);
int hal_mtx_unlock(hal_mtx_t *mtx);

int hal_thrd_create(hal_thrd_t *thr, thrd_start_t func, void *arg);
hal_thrd_t hal_thrd_current(void);
int hal_thrd_detach(hal_thrd_t thr);
int hal_thrd_equal(hal_thrd_t thr0, hal_thrd_t thr1);
void hal_thrd_exit(int res);
int hal_thrd_join(hal_thrd_t thr, int *res);
void hal_thrd_sleep(const hal_thrd_timeout *xt);
void hal_thrd_yield(void);

int hal_tss_create(hal_tss_t *key, tss_dtor_t dtor);
void hal_tss_delete(hal_tss_t key);
void* hal_tss_get(hal_tss_t key);
int hal_tss_set(hal_tss_t key, void *val);

int hal_timeout(hal_thrd_timeout *xt, int base);

#ifdef __cplusplus
}
#endif
#endif // HAL_THREADS_HEAD
