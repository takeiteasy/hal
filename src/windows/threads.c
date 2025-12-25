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

#ifndef HAL_NO_THREADS
#include "hal/threads.h"
#include <windows.h>
#include <time.h>

static void impl_tss_dtor_invoke();  // forward decl.

struct impl_thrd_param {
    thrd_start_t func;
    void *arg;
};

static unsigned __stdcall impl_thrd_routine(void *p) {
    struct impl_thrd_param pack;
    int code;
    memcpy(&pack, p, sizeof(struct impl_thrd_param));
    free(p);
    code = pack.func(pack.arg);
    impl_tss_dtor_invoke();
    return (unsigned)code;
}

static DWORD impl_xtime2msec(const hal_thrd_timeout *xt) {
    return (DWORD)((xt->sec * 1000u) + (xt->nsec / 1000000));
}

#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
struct impl_call_once_param { void (*func)(void); };

static BOOL CALLBACK impl_call_once_callback(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context) {
    struct impl_call_once_param *param = (struct impl_call_once_param*)Parameter;
    (param->func)();
    ((void)InitOnce);
    ((void)Context);  // suppress warning
    return TRUE;
}
#endif  // ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE

#ifndef EMULATED_THREADS_USE_NATIVE_CV
/*
Note:
  The implementation of condition variable is ported from Boost.Interprocess
  See http://www.boost.org/boost/interprocess/sync/windows/condition.hpp
*/
static void impl_cond_do_signal(hal_cnd_t *cond, int broadcast) {
    int nsignal = 0;

    EnterCriticalSection(&cond->monitor);
    if (cond->to_unblock != 0) {
        if (cond->blocked == 0) {
            LeaveCriticalSection(&cond->monitor);
            return;
        }
        if (broadcast) {
            cond->to_unblock += nsignal = cond->blocked;
            cond->blocked = 0;
        } else {
            nsignal = 1;
            cond->to_unblock++;
            cond->blocked--;
        }
    } else if (cond->blocked > cond->gone) {
        WaitForSingleObject(cond->sem_gate, INFINITE);
        if (cond->gone != 0) {
            cond->blocked -= cond->gone;
            cond->gone = 0;
        }
        if (broadcast) {
            nsignal = cond->to_unblock = cond->blocked;
            cond->blocked = 0;
        } else {
            nsignal = cond->to_unblock = 1;
            cond->blocked--;
        }
    }
    LeaveCriticalSection(&cond->monitor);

    if (0 < nsignal)
        ReleaseSemaphore(cond->sem_queue, nsignal, NULL);
}

static int impl_cond_do_wait(hal_cnd_t *cond, hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    int nleft = 0;
    int ngone = 0;
    int timeout = 0;
    DWORD w;

    WaitForSingleObject(cond->sem_gate, INFINITE);
    cond->blocked++;
    ReleaseSemaphore(cond->sem_gate, 1, NULL);

    mtx_unlock(mtx);

    w = WaitForSingleObject(cond->sem_queue, xt ? impl_xtime2msec(xt) : INFINITE);
    timeout = (w == WAIT_TIMEOUT);

    EnterCriticalSection(&cond->monitor);
    if ((nleft = cond->to_unblock) != 0) {
        if (timeout) {
            if (cond->blocked != 0)
                cond->blocked--;
            else
                cond->gone++;
        }
        if (--cond->to_unblock == 0) {
            if (cond->blocked != 0) {
                ReleaseSemaphore(cond->sem_gate, 1, NULL);
                nleft = 0;
            }
            else if ((ngone = cond->gone) != 0)
                cond->gone = 0;
        }
    } else if (++cond->gone == INT_MAX/2) {
        WaitForSingleObject(cond->sem_gate, INFINITE);
        cond->blocked -= cond->gone;
        ReleaseSemaphore(cond->sem_gate, 1, NULL);
        cond->gone = 0;
    }
    LeaveCriticalSection(&cond->monitor);

    if (nleft == 1) {
        while (ngone--)
            WaitForSingleObject(cond->sem_queue, INFINITE);
        ReleaseSemaphore(cond->sem_gate, 1, NULL);
    }

    mtx_lock(mtx);
    return timeout ? HAL_THRD_BUSY : HAL_THRD_SUCCESS;
}
#endif  // ifndef EMULATED_THREADS_USE_NATIVE_CV

static struct impl_tss_dtor_entry {
    hal_tss_t key;
    tss_dtor_t dtor;
} impl_tss_dtor_tbl[EMULATED_THREADS_TSS_DTOR_SLOTNUM];

static int impl_tss_dtor_register(hal_tss_t key, tss_dtor_t dtor) {
    int i;
    for (i = 0; i < EMULATED_THREADS_TSS_DTOR_SLOTNUM; i++) {
        if (!impl_tss_dtor_tbl[i].dtor)
            break;
    }
    if (i == EMULATED_THREADS_TSS_DTOR_SLOTNUM)
        return 1;
    impl_tss_dtor_tbl[i].key = key.key;
    impl_tss_dtor_tbl[i].dtor = dtor;
    return 0;
}

static void impl_tss_dtor_invoke() {
    int i;
    for (i = 0; i < EMULATED_THREADS_TSS_DTOR_SLOTNUM; i++) {
        if (impl_tss_dtor_tbl[i].dtor) {
            void* val = tss_get(impl_tss_dtor_tbl[i].key);
            if (val)
                (impl_tss_dtor_tbl[i].dtor)(val);
        }
    }
}

bool hal_threads_available(void) {
    return true;
}

void hal_call_once(hal_once_flag *flag, void (*func)(void)) {
    assert(flag && func);
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
    struct impl_call_once_param param;
    param.func = func;
    InitOnceExecuteOnce(flag->status, impl_call_once_callback, (PVOID)&param, NULL);
#else
    if (InterlockedCompareExchange(&flag->status, 1, 0) == 0) {
        (func)();
        InterlockedExchange(&flag->status, 2);
    } else {
        while (flag->status == 1)
            thrd_yield(); // busy loop!
    }
#endif
}

int hal_cnd_broadcast(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    WakeAllConditionVariable(&cond->condvar);
#else
    impl_cond_do_signal(cond, 1);
#endif
    return HAL_THRD_SUCCESS;
}

void hal_cnd_destroy(hal_cnd_t *cond) {
    assert(cond);
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    // do nothing
#else
    CloseHandle(cond->sem_queue);
    CloseHandle(cond->sem_gate);
    DeleteCriticalSection(&cond->monitor);
#endif
}

int hal_cnd_init(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    InitializeConditionVariable(&cond->condvar);
#else
    cond->blocked = 0;
    cond->gone = 0;
    cond->to_unblock = 0;
    cond->sem_queue = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    cond->sem_gate = CreateSemaphore(NULL, 1, 1, NULL);
    InitializeCriticalSection(&cond->monitor);
#endif
    return HAL_THRD_SUCCESS;
}

int hal_cnd_signal(hal_cnd_t *cond) {
    if (!cond)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    WakeConditionVariable(&cond->condvar);
#else
    impl_cond_do_signal(cond, 0);
#endif
    return HAL_THRD_SUCCESS;
}

// 7.25.3.5
int hal_cnd_timedwait(hal_cnd_t *cond, hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    if (!cond || !mtx || !xt)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    if (SleepConditionVariableCS(&cond->condvar, &mtx->cs, impl_xtime2msec(xt)))
        return HAL_THRD_SUCCESS;
    return (GetLastError() == ERROR_TIMEOUT) ? HAL_THRD_BUSY : HAL_THRD_ERROR;
#else
    return impl_cond_do_wait(cond, mtx, xt);
#endif
}

int hal_cnd_wait(hal_cnd_t *cond, hal_mtx_t *mtx) {
    if (!cond || !mtx)
        return HAL_THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
    SleepConditionVariableCS(&cond->condvar, &mtx->cs, INFINITE);
#else
    impl_cond_do_wait(cond, mtx, NULL);
#endif
    return HAL_THRD_SUCCESS;
}

void hal_mtx_destroy(hal_mtx_t *mtx) {
    assert(mtx);
    DeleteCriticalSection(&mtx->cs);
}

int hal_mtx_init(hal_mtx_t *mtx, int type) {
    if (!mtx)
        return HAL_THRD_ERROR;
    if (type != HAL_MTX_PLAIN && type != HAL_MTX_TIMED && type != HAL_MTX_TRY
        && type != (HAL_MTX_PLAIN | HAL_MTX_RECURSIVE)
        && type != (HAL_MTX_TIMED | HAL_MTX_RECURSIVE)
        && type != (HAL_MTX_TRY   | HAL_MTX_RECURSIVE))
        return HAL_THRD_ERROR;
    InitializeCriticalSection(&mtx->cs);
    return HAL_THRD_SUCCESS;
}

int hal_mtx_lock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    EnterCriticalSection(&mtx->cs);
    return HAL_THRD_SUCCESS;
}

int hal_mtx_timedlock(hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    time_t expire, now;
    if (!mtx || !xt)
        return HAL_THRD_ERROR;
    expire = time(NULL);
    expire += xt->sec;
    while (HAL_MTX_TRYlock(mtx) != HAL_THRD_SUCCESS) {
        now = time(NULL);
        if (expire < now)
            return HAL_THRD_BUSY;
        // busy loop!
        thrd_yield();
    }
    return HAL_THRD_SUCCESS;
}

int hal_mtx_trylock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    return TryEnterCriticalSection(&mtx->cs) ? HAL_THRD_SUCCESS : HAL_THRD_BUSY;
}

int hal_mtx_unlock(hal_mtx_t *mtx) {
    if (!mtx)
        return HAL_THRD_ERROR;
    LeaveCriticalSection(&mtx->cs);
    return HAL_THRD_SUCCESS;
}

int hal_thrd_create(hal_thrd_t *thr, thrd_start_t func, void *arg) {
    struct impl_thrd_param *pack;
    uintptr_t handle;
    if (!thr || !thr->hndl)
        return HAL_THRD_ERROR;
    if (!(pack = malloc(sizeof(struct impl_thrd_param))))
        return HAL_THRD_NOMEM;
    pack->func = func;
    pack->arg = arg;
    handle = _beginthreadex(NULL, 0, impl_thrd_routine, pack, 0, NULL);
    if (handle == 0) {
        if (errno == EAGAIN || errno == EACCES)
            return HAL_THRD_NOMEM;
        return HAL_THRD_ERROR;
    }
    *thr->hndl = handle;
    return HAL_THRD_SUCCESS;
}

hal_thrd_t hal_thrd_current(void) {
    return (hal_thrd_t){.hndl=GetCurrentThread()};
}

int hal_thrd_detach(hal_thrd_t thr) {
    CloseHandle(thr.hndl);
    return HAL_THRD_SUCCESS;
}

int hal_thrd_equal(hal_thrd_t thr0, hal_thrd_t thr1) {
    return thr0.hndl == thr1.hndl;
}

void hal_thrd_exit(int res) {
    impl_tss_dtor_invoke();
    _endthreadex((unsigned)res);
}

int hal_thrd_join(hal_thrd_t thr, int *res) {
    DWORD w, code;
    w = WaitForSingleObject(thr.hndl, INFINITE);
    if (w != WAIT_OBJECT_0)
        return HAL_THRD_ERROR;
    if (res) {
        if (!GetExitCodeThread(thr.hndl, &code)) {
            CloseHandle(thr.hndl);
            return HAL_THRD_ERROR;
        }
        *res = (int)code;
    }
    CloseHandle(thr.hndl);
    return HAL_THRD_SUCCESS;
}

void hal_thrd_sleep(const hal_thrd_timeout *xt) {
    assert(xt);
    Sleep(impl_xtime2msec(xt));
}

void hal_thrd_yield(void) {
    SwitchToThread();
}

int hal_tss_create(hal_tss_t *key, tss_dtor_t dtor) {
    if (!key)
        return HAL_THRD_ERROR;
    key->key = TlsAlloc();
    if (dtor) {
        if (impl_tss_dtor_register(key->key, dtor)) {
            TlsFree(key->key);
            return HAL_THRD_ERROR;
        }
    }
    return key->key != 0xFFFFFFFF ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
}

void hal_tss_delete(hal_tss_t key) {
    TlsFree(key.key);
}

void* hal_tss_get(hal_tss_t key) {
    return TlsGetValue(key.key);
}

int hal_tss_set(hal_tss_t key, void *val) {
    return TlsSetValue(key.key, val) ? HAL_THRD_SUCCESS : HAL_THRD_ERROR;
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
#endif // HAL_NO_THREADS
