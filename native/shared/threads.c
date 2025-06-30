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

struct impl_thrd_param {
    thrd_start_t func;
    void *arg;
};

void *impl_thrd_routine(void *p) {
    struct impl_thrd_param pack = *((struct impl_thrd_param *)p);
    free(p);
    return (void*)pack.func(pack.arg);
}

bool hal_threads_available(void) {
    return true;
}

void hal_call_once(hal_once_flag *flag, void (*func)(void)) {
    pthread_once(&flag->flag, func);
}

int hal_cnd_broadcast(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
    pthread_cond_broadcast(&cond->cnd);
    return HAL_THRD_SUCCESS;
}

void hal_cnd_destroy(hal_cnd_t *cond) {
    assert(cond);
    pthread_cond_destroy(&cond->cnd);
}

int hal_cnd_init(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
    pthread_cond_init(&cond->cnd, NULL);
    return HAL_THRD_SUCCESS;
}

int hal_cnd_signal(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
    pthread_cond_signal(&cond->cnd);
    return HAL_THRD_SUCCESS;
}

int hal_cnd_timedwait(hal_cnd_t *cond, hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    struct timespec abs_time;
    int rt;
    if (!cond || !mtx || !xt)
        return HAL_THRD_ERROR;
    rt = pthread_cond_timedwait(&cond->cnd, &mtx->mtx, &abs_time);
    if (rt == ETIMEDOUT)
        return HAL_THRD_BUSY;
    return rt == 0 ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
}

int hal_cnd_wait(hal_cnd_t *cond, hal_mtx_t *mtx) {
    if (!cond || !mtx)
        return HAL_THRD_ERROR;
    pthread_cond_wait(&cond->cnd, &mtx->mtx);
    return HAL_THRD_SUCCESS;
}

void hal_mtx_destroy(hal_mtx_t *mtx) {
    assert(mtx);
    pthread_mutex_destroy(&mtx->mtx);
}

int hal_mtx_init(hal_mtx_t *mtx, int type) {
    pthread_mutexattr_t attr;
    if (!mtx)
        return HAL_THRD_ERROR;
    if (type != HAL_MTX_PLAIN && type != HAL_MTX_TIMED && type != HAL_MTX_TRY
        && type != (HAL_MTX_PLAIN | HAL_MTX_RECURSIVE)
        && type != (HAL_MTX_TIMED | HAL_MTX_RECURSIVE)
        && type != (HAL_MTX_TRY   | HAL_MTX_RECURSIVE))
        return HAL_THRD_ERROR;
    pthread_mutexattr_init(&attr);
    if ((type & HAL_MTX_RECURSIVE) != 0) {
#if defined(__linux__) || defined(__linux)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
    }
    pthread_mutex_init(&mtx->mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return HAL_THRD_SUCCESS;
}

int hal_mtx_lock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    pthread_mutex_lock(&mtx->mtx);
    return HAL_THRD_SUCCESS;
}

int hal_mtx_timedlock(hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    if (!mtx || !xt)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
    struct timespec ts;
    int rt;
    ts.tv_sec = xt->sec;
    ts.tv_nsec = xt->nsec;
    rt = pthread_mutex_timedlock(&mtx->mtx, &ts);
    if (rt == 0)
        return HAL_THRD_SUCCESS;
    return rt == ETIMEDOUT ? HAL_THRD_BUSY : HAL_THRD_ERROR;
#else
    time_t expire = time(NULL);
    expire += xt->sec;
    while (hal_mtx_trylock(mtx) != HAL_THRD_SUCCESS) {
        time_t now = time(NULL);
        if (expire < now)
            return HAL_THRD_BUSY;
        // busy loop!
        hal_thrd_yield();
    }
    return HAL_THRD_SUCCESS;
#endif
}

int hal_mtx_trylock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    return pthread_mutex_trylock(&mtx->mtx) == 0 ? HAL_THRD_SUCCESS : HAL_THRD_BUSY;
}

int hal_mtx_unlock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    pthread_mutex_unlock(&mtx->mtx);
    return HAL_THRD_SUCCESS;
}

int hal_thrd_create(hal_thrd_t *thr, thrd_start_t func, void *arg) {
    struct impl_thrd_param *pack;
    if (!thr)
        return HAL_THRD_ERROR;
    if (!(pack = malloc(sizeof(struct impl_thrd_param))))
        return HAL_THRD_NOMEM;
    pack->func = func;
    pack->arg = arg;
    if (pthread_create(&thr->thrd, NULL, impl_thrd_routine, pack) != 0) {
        free(pack);
        return HAL_THRD_ERROR;
    }
    return HAL_THRD_SUCCESS;
}

hal_thrd_t hal_thrd_current(void) {
    return (hal_thrd_t){.thrd=pthread_self()};
}

int hal_thrd_detach(hal_thrd_t thr) {
    return pthread_detach(thr.thrd) == 0 ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
}

int hal_thrd_equal(hal_thrd_t thr0, hal_thrd_t thr1) {
    return pthread_equal(thr0.thrd, thr1.thrd);
}

void hal_thrd_exit(int res) {
    pthread_exit((void*)res);
}

int hal_thrd_join(hal_thrd_t thr, int *res) {
    void *code;
    if (pthread_join(thr.thrd, &code) != 0)
        return HAL_THRD_ERROR;
    if (res)
        *res = (int)code;
    return HAL_THRD_SUCCESS;
}

void hal_thrd_sleep(const hal_thrd_timeout *xt) {
    struct timespec req;
    assert(xt);
    req.tv_sec = xt->sec;
    req.tv_nsec = xt->nsec;
    nanosleep(&req, NULL);
}

void hal_thrd_yield(void) {
    sched_yield();
}

int hal_tss_create(hal_tss_t *key, tss_dtor_t dtor) {
    if (!key)
        return HAL_THRD_ERROR;
    return pthread_key_create(&key->key, dtor) == 0 ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
}

void hal_tss_delete(hal_tss_t key) {
    pthread_key_delete(key.key);
}

void* hal_tss_get(hal_tss_t key) {
    return pthread_getspecific(key.key);
}

int hal_tss_set(hal_tss_t key, void *val) {
    return pthread_setspecific(key.key, val) == 0 ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
}

int hal_timeout(hal_thrd_timeout *xt, int base) {
    if (!xt)
        return 0;
    if (base == TIME_UTC) {
        xt->sec = time(NULL);
        xt->nsec = 0;
        return base;
    }
    return 0;
}
