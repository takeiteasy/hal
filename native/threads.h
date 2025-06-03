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

typedef void (*tss_dtor_t)(void*);
typedef int (*thrd_start_t)(void*);

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

typedef struct paul_cnd_t {
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
} paul_cnd_t;

typdef struct paul_thrd_t {
    HANDLE hndl;
} paul_thrd_t;

typedef struct paul_tss_t {
    DWORD key;
} paul_tss_t;

typedef struct paul_mtx_t {
    CRITICAL_SECTION cs;
} paul_mtx_t;

typedef struct paul_once_flag {
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
    INIT_ONCE status;
#else
    volatile LONG status;
#endif
} paul_once_flag;
#else
#include <pthread.h>

typedef struct paul_cnd_t {
    pthread_cond_t cnd;
} paul_cnd_t;

typedef struct paul_thrd_t {
    pthread_t thrd;
} paul_thrd_t;

typedef struct paul_tss_t {
    pthread_key_t key;
} paul_tss_t;

typedef struct paul_mtx_t {
    pthread_mutex_t mtx;
} paul_mtx_t;

typedef struct paul_once_flag {
    pthread_once_t flag;
} paul_once_flag;

#endif

#include <time.h>

typedef struct paul_thrd_timeout {
    time_t sec;
    long nsec;
} paul_thrd_timeout;

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
