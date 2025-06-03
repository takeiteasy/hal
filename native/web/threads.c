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

#include "../threads.h"
#ifndef __EMSCRIPTEN_PTHREADS__
#warning emscripten is not targeting pthreads, see https://emscripten.org/docs/porting/pthreads.html

void paul_call_once(paul_once_flag *flag, void (*func)(void)) {}

int paul_cnd_broadcast(paul_cnd_t *cond) {
    return -1;
}

void paul_cnd_destroy(paul_cnd_t *cond) {}

int paul_cnd_init(paul_cnd_t *cond) {
    return -1;
}

int paul_cnd_signal(paul_cnd_t *cond) {
    return -1;
}

int paul_cnd_timedwait(paul_cnd_t *cond, paul_mtx_t *mtx, const paul_thrd_timeout *xt) {
    return -1;
}

int paul_cnd_wait(paul_cnd_t *cond, paul_mtx_t *mtx) {
    return -1;
}

void paul_mtx_destroy(paul_mtx_t *mtx) {}

int paul_mtx_init(paul_mtx_t *mtx, int type) {
    return -1;
}

int paul_mtx_lock(paul_mtx_t *mtx) {
    return -1;
}

int paul_mtx_timedlock(paul_mtx_t *mtx, const paul_thrd_timeout *xt) {
    return -1;
}

int paul_mtx_trylock(paul_mtx_t *mtx) {
    return -1;
}

int paul_mtx_unlock(paul_mtx_t *mtx) {
    return -1;
}

int paul_thrd_create(paul_thrd_t *thr, thrd_start_t func, void *arg) {
    return -1;
}

paul_thrd_t paul_thrd_current(void) {
    return (paul_thrd_t){.thrd={0}};
}

int paul_thrd_detach(paul_thrd_t thr) {
    return -1;
}

int paul_thrd_equal(paul_thrd_t thr0, paul_thrd_t thr1) {
    return -1;
}

void paul_thrd_exit(int res) {}

int paul_thrd_join(paul_thrd_t thr, int *res) {
    return -1;
}

void paul_thrd_sleep(const paul_thrd_timeout *xt) {}

void paul_thrd_yield(void) {}

int paul_tss_create(paul_tss_t *key, tss_dtor_t dtor) {
    return -1;
}

void paul_tss_delete(paul_tss_t key) {}

void* paul_tss_get(paul_tss_t key) {
    return NULL;
}

int paul_tss_set(paul_tss_t key, void *val) {
    return -1;
}

int paul_timeout(paul_thrd_timeout *xt, int base) {
    return -1;
}
#else
#include "../shared/threads.c"
#endif
