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
#include "../threads.h"
#ifndef __EMSCRIPTEN_PTHREADS__
#warning emscripten is not targeting pthreads, see https://emscripten.org/docs/porting/pthreads.html

bool hal_threads_available(void) {
    return false;
}

void hal_call_once(hal_once_flag *flag, void (*func)(void)) {}

int hal_cnd_broadcast(hal_cnd_t *cond) {
    return -1;
}

void hal_cnd_destroy(hal_cnd_t *cond) {}

int hal_cnd_init(hal_cnd_t *cond) {
    return -1;
}

int hal_cnd_signal(hal_cnd_t *cond) {
    return -1;
}

int hal_cnd_timedwait(hal_cnd_t *cond, hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    return -1;
}

int hal_cnd_wait(hal_cnd_t *cond, hal_mtx_t *mtx) {
    return -1;
}

void hal_mtx_destroy(hal_mtx_t *mtx) {}

int hal_mtx_init(hal_mtx_t *mtx, int type) {
    return -1;
}

int hal_mtx_lock(hal_mtx_t *mtx) {
    return -1;
}

int hal_mtx_timedlock(hal_mtx_t *mtx, const hal_thrd_timeout *xt) {
    return -1;
}

int hal_mtx_trylock(hal_mtx_t *mtx) {
    return -1;
}

int hal_mtx_unlock(hal_mtx_t *mtx) {
    return -1;
}

int hal_thrd_create(hal_thrd_t *thr, thrd_start_t func, void *arg) {
    return -1;
}

hal_thrd_t hal_thrd_current(void) {
    return (hal_thrd_t){.thrd={0}};
}

int hal_thrd_detach(hal_thrd_t thr) {
    return -1;
}

int hal_thrd_equal(hal_thrd_t thr0, hal_thrd_t thr1) {
    return -1;
}

void hal_thrd_exit(int res) {}

int hal_thrd_join(hal_thrd_t thr, int *res) {
    return -1;
}

void hal_thrd_sleep(const hal_thrd_timeout *xt) {}

void hal_thrd_yield(void) {}

int hal_tss_create(hal_tss_t *key, tss_dtor_t dtor) {
    return -1;
}

void hal_tss_delete(hal_tss_t key) {}

void* hal_tss_get(hal_tss_t key) {
    return NULL;
}

int hal_tss_set(hal_tss_t key, void *val) {
    return -1;
}

int hal_timeout(hal_thrd_timeout *xt, int base) {
    return -1;
}
#else
#include "../shared/threads.c"
#endif
#endif // HAL_NO_THREADS
