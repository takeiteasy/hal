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

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#ifdef INIT_ONCE_STATIC_INIT
#define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
#else
#define TSS_DTOR_ITERATIONS 1  // assume TSS dtor MAY be called at least once.
#endif

#if !defined(__CYGWIN__) && !defined(PLATFORM_MAC)
#define EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
#endif

struct paul_cnd_t {
    pthread_cond_t cnd;
};

struct paul_thrd_t {
    pthread_t thrd;
};

struct paul_tss_t {
    pthread_key_t key;
};

struct paul_mtx_t {
    pthread_mutex_t mtx;
};

struct paul_once_flag {
    pthread_once_t flag;
};

struct impl_thrd_param {
    thrd_start_t func;
    void *arg;
};

struct paul_thrd_timeout {
    time_t sec;
    long nsec;
};

void *impl_thrd_routine(void *p)
{
    struct impl_thrd_param pack = *((struct impl_thrd_param *)p);
    free(p);
    return (void*)pack.func(pack.arg);
}

void paul_call_once(paul_once_flag *flag, void (*func)(void)) {
    pthread_once(&flag->flag, func);
}

int paul_cnd_broadcast(paul_cnd_t *cond) {
    if (!cond)
        return PAUL_THRD_ERROR;
    pthread_cond_broadcast(&cond->cnd);
    return PAUL_THRD_SUCCESS;
}

void paul_cnd_destroy(paul_cnd_t *cond) {
    assert(cond);
    pthread_cond_destroy(&cond->cnd);
}

int paul_cnd_init(paul_cnd_t *cond) {
    if (!cond)
        return PAUL_THRD_ERROR;
    pthread_cond_init(&cond->cnd, NULL);
    return PAUL_THRD_SUCCESS;
}

int paul_cnd_signal(paul_cnd_t *cond) {
    if (!cond)
        return PAUL_THRD_ERROR;
    pthread_cond_signal(&cond->cnd);
    return PAUL_THRD_SUCCESS;
}

int paul_cnd_timedwait(paul_cnd_t *cond, paul_mtx_t *mtx, const paul_thrd_timeout *xt) {
    struct timespec abs_time;
    int rt;
    if (!cond || !mtx || !xt)
        return PAUL_THRD_ERROR;
    rt = pthread_cond_timedwait(&cond->cnd, &mtx->mtx, &abs_time);
    if (rt == ETIMEDOUT)
        return PAUL_THRD_BUSY;
    return rt == 0 ? PAUL_THRD_SUCCESS : PAUL_THRD_ERROR;
}

int paul_cnd_wait(paul_cnd_t *cond, paul_mtx_t *mtx) {
    if (!cond || !mtx)
        return PAUL_THRD_ERROR;
    pthread_cond_wait(&cond->cnd, &mtx->mtx);
    return PAUL_THRD_SUCCESS;
}

void paul_mtx_destroy(paul_mtx_t *mtx) {
    assert(mtx);
    pthread_mutex_destroy(&mtx->mtx);
}

int paul_mtx_init(paul_mtx_t *mtx, int type) {
    pthread_mutexattr_t attr;
    if (!mtx)
        return PAUL_THRD_ERROR;
    if (type != PAUL_MTX_PLAIN && type != PAUL_MTX_TIMED && type != PAUL_MTX_TRY
        && type != (PAUL_MTX_PLAIN | PAUL_MTX_RECURSIVE)
        && type != (PAUL_MTX_TIMED | PAUL_MTX_RECURSIVE)
        && type != (PAUL_MTX_TRY   | PAUL_MTX_RECURSIVE))
        return PAUL_THRD_ERROR;
    pthread_mutexattr_init(&attr);
    if ((type & PAUL_MTX_RECURSIVE) != 0) {
#if defined(__linux__) || defined(__linux)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
    }
    pthread_mutex_init(&mtx->mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return PAUL_THRD_SUCCESS;
}

int paul_mtx_lock(paul_mtx_t *mtx) {
    if (!mtx)
        return PAUL_THRD_ERROR;
    pthread_mutex_lock(&mtx->mtx);
    return PAUL_THRD_SUCCESS;
}

int paul_mtx_timedlock(paul_mtx_t *mtx, const paul_thrd_timeout *xt) {
    if (!mtx || !xt)
        return PAUL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
    struct timespec ts;
    int rt;
    ts.tv_sec = xt->sec;
    ts.tv_nsec = xt->nsec;
    rt = pthread_mutex_timedlock(&mtx->mtx, &ts);
    if (rt == 0)
        return PAUL_THRD_SUCCESS;
    return rt == ETIMEDOUT ? PAUL_THRD_BUSY : PAUL_THRD_ERROR;
#else
    time_t expire = time(NULL);
    expire += xt->sec;
    while (paul_mtx_trylock(mtx) != PAUL_THRD_SUCCESS) {
        time_t now = time(NULL);
        if (expire < now)
            return PAUL_THRD_BUSY;
        // busy loop!
        paul_thrd_yield();
    }
    return PAUL_THRD_SUCCESS;
#endif
}

int paul_mtx_trylock(paul_mtx_t *mtx) {
    if (!mtx)
        return PAUL_THRD_ERROR;
    return pthread_mutex_trylock(&mtx->mtx) == 0 ? PAUL_THRD_SUCCESS : PAUL_THRD_BUSY;
}

int paul_mtx_unlock(paul_mtx_t *mtx) {
    if (!mtx)
        return PAUL_THRD_ERROR;
    pthread_mutex_unlock(&mtx->mtx);
    return PAUL_THRD_SUCCESS;
}

int paul_thrd_create(paul_thrd_t *thr, thrd_start_t func, void *arg) {
    struct impl_thrd_param *pack;
    if (!thr)
        return PAUL_THRD_ERROR;
    if (!(pack = malloc(sizeof(struct impl_thrd_param))))
        return PAUL_THRD_NOMEM;
    pack->func = func;
    pack->arg = arg;
    if (pthread_create(&thr->thrd, NULL, impl_thrd_routine, pack) != 0) {
        free(pack);
        return PAUL_THRD_ERROR;
    }
    return PAUL_THRD_SUCCESS;
}

paul_thrd_t paul_thrd_current(void) {
    return (paul_thrd_t){.thrd=pthread_self()};
}

int paul_thrd_detach(paul_thrd_t thr) {
    return pthread_detach(thr.thrd) == 0 ? PAUL_THRD_SUCCESS : PAUL_THRD_ERROR;
}

int paul_thrd_equal(paul_thrd_t thr0, paul_thrd_t thr1) {
    return pthread_equal(thr0.thrd, thr1.thrd);
}

void paul_thrd_exit(int res) {
    pthread_exit((void*)res);
}

int paul_thrd_join(paul_thrd_t thr, int *res) {
    void *code;
    if (pthread_join(thr.thrd, &code) != 0)
        return PAUL_THRD_ERROR;
    if (res)
        *res = (int)code;
    return PAUL_THRD_SUCCESS;
}

void paul_thrd_sleep(const paul_thrd_timeout *xt) {
    struct timespec req;
    assert(xt);
    req.tv_sec = xt->sec;
    req.tv_nsec = xt->nsec;
    nanosleep(&req, NULL);
}

void paul_thrd_yield(void) {
    sched_yield();
}

int paul_tss_create(paul_tss_t *key, tss_dtor_t dtor) {
    if (!key)
        return PAUL_THRD_ERROR;
    return pthread_key_create(&key->key, dtor) == 0 ? PAUL_THRD_SUCCESS : PAUL_THRD_ERROR;
}

void paul_tss_delete(paul_tss_t key) {
    pthread_key_delete(key.key);
}

void* paul_tss_get(paul_tss_t key) {
    return pthread_getspecific(key.key);
}

int paul_tss_set(paul_tss_t key, void *val) {
    return pthread_setspecific(key.key, val) == 0 ? PAUL_THRD_SUCCESS : PAUL_THRD_ERROR;
}

int paul_timeout(paul_thrd_timeout *xt, int base) {
    if (!xt)
        return 0;
    if (base == TIME_UTC) {
        xt->sec = time(NULL);
        xt->nsec = 0;
        return base;
    }
    return 0;
}
