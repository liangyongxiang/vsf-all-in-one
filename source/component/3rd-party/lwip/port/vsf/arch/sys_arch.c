/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/

#include "component/tcpip/vsf_tcpip_cfg.h"

#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED

#define __VSF_EDA_CLASS_INHERIT__
#include "kernel/vsf_kernel.h"

#include "./cc.h"
#include "./sys_arch.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/*============================ MACROS ========================================*/

#if VSF_KERNEL_CFG_EDA_SUPPORT_ON_TERMINATE != ENABLED
#   error "VSF_KERNEL_CFG_EDA_SUPPORT_ON_TERMINATE is required"
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*
struct vsf_lwip_thread_t {
    implement(vsf_thread_t)

    void *arg;
    lwip_thread_fn fn;
};
typedef struct vsf_lwip_thread_t vsf_lwip_thread_t;
*/

dcl_vsf_thread_ex(vsf_lwip_thread_t)
def_vsf_thread_ex(vsf_lwip_thread_t,
    def_params(
        void *arg;
        lwip_thread_fn lwip_thread;
    )
)


/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if SYS_LIGHTWEIGHT_PROT
sys_prot_t sys_arch_protect(void)
{
    return vsf_protect_int();
}

void sys_arch_unprotect(sys_prot_t pval)
{
    vsf_unprotect_int(pval);
}
#endif

u32_t sys_now(void)
{
    return vsf_systimer_get_ms();
}

u32_t sys_jiffies(void)
{
    return (u32_t)vsf_systimer_get_ms();
}

#if !NO_SYS

#   if LWIP_TCPIP_CORE_LOCKING

static vsf_eda_t *__lwip_core_lock_holder_thread;
extern sys_mutex_t lock_tcpip_core;

void sys_lock_tcpip_core(void)
{
    sys_mutex_lock(&lock_tcpip_core);
    __lwip_core_lock_holder_thread = vsf_eda_get_cur();
}

void sys_unlock_tcpip_core(void)
{
    __lwip_core_lock_holder_thread = NULL;
    sys_mutex_unlock(&lock_tcpip_core);
}
#   endif

static vsf_eda_t *__lwip_tcpip_thread;

void sys_mark_tcpip_thread(void)
{
    __lwip_tcpip_thread = vsf_eda_get_cur();
}

void sys_check_core_locking(void)
{
    /* Embedded systems should check we are NOT in an interrupt context here */

    if (__lwip_tcpip_thread != 0) {
        vsf_eda_t *current_thread = vsf_eda_get_cur();
        if (current_thread != __lwip_core_lock_holder_thread) {
            __asm("nop");
        }

#if LWIP_TCPIP_CORE_LOCKING
        LWIP_ASSERT("Function called without core lock", current_thread == __lwip_core_lock_holder_thread);
#else /* LWIP_TCPIP_CORE_LOCKING */
        LWIP_ASSERT("Function called from wrong thread", current_thread == __lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
        LWIP_UNUSED_ARG(current_thread); /* for LWIP_NOASSERT */
    }
}

// thread
static void __vsf_lwip_thread_on_terminate(vsf_eda_t *eda)
{
    vsf_heap_free(eda);
}

implement_vsf_thread_ex(vsf_lwip_thread_t)
{
    vsf_this.lwip_thread(vsf_this.arg);
}
/*
static void __vsf_lwip_thread_entry(vsf_thread_t *thread)
{
    sys_thread_t sys_thread = (sys_thread_t)thread;
    sys_thread->fn(sys_thread->arg);
}
*/
sys_thread_t sys_thread_new(const char *name,
                            lwip_thread_fn fn,
                            void *arg,
                            int stacksize,
                            int prio)
{
    sys_thread_t thread;
    uint_fast32_t thread_size = sizeof(*thread);
    uint64_t *stack;

    thread_size += (1 << VSF_KERNEL_CFG_THREAD_STACK_ALIGN_BIT) - 1;
    thread_size &= ~((1 << VSF_KERNEL_CFG_THREAD_STACK_ALIGN_BIT) - 1);
    stacksize   += (1 << VSF_KERNEL_CFG_THREAD_STACK_ALIGN_BIT) - 1;
    stacksize   &= ~((1 << VSF_KERNEL_CFG_THREAD_STACK_ALIGN_BIT) - 1);

    thread = vsf_heap_malloc_aligned(thread_size + stacksize,
                        1 << VSF_KERNEL_CFG_THREAD_STACK_ALIGN_BIT);
    if (NULL == thread) {
        return NULL;
    }

    thread->on_terminate = __vsf_lwip_thread_on_terminate;

    thread->arg = arg;
    thread->lwip_thread = fn;

    stack = (uint64_t *)((uintptr_t)thread + thread_size);
    init_vsf_thread_ex( vsf_lwip_thread_t,
                        thread,
                        prio,
                        stack,
                        stacksize);
    /*
    thread->stack = (uint64_t *)((uintptr_t)thread + thread_size);
    thread->stack_size = stacksize;

    vsf_thread_start(&(thread->use_as__vsf_thread_t), prio);
    */
    return thread;
}

// sem
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    vsf_eda_sem_init(sem, count);
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{

}

int sys_sem_valid(sys_sem_t *sem)
{
    return sem->max_union.max_value != 0 ? 1 : 0;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    sem->max_union.bits.max = 0;
}

void sys_sem_signal(sys_sem_t *sem)
{
    vsf_eda_sem_post(sem);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    vsf_systimer_tick_t pre = vsf_systimer_get_tick();
    vsf_timeout_tick_t timeout_tick = (0 == timeout) ? -1 : vsf_systimer_ms_to_tick(timeout);
    vsf_sync_reason_t reason = vsf_thread_sem_pend(sem, timeout_tick);
    if (VSF_SYNC_GET == reason) {
        pre = vsf_systimer_get_tick() - pre;
        return vsf_systimer_tick_to_ms(pre);
    } else {
        return SYS_ARCH_TIMEOUT;
    }
}

// mutex
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    vsf_eda_mutex_init(mutex);
    return ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{

}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    vsf_thread_mutex_enter(mutex, -1);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    vsf_thread_mutex_leave(mutex);
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
    return mutex->cur_union.bits.has_owner ? 1 : 0;
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    mutex->cur_union.bits.has_owner = 0;
}

// mbox
static void __sys_mbox_next(sys_mbox_t *mbox, uint16_t *pos)
{
    if (++(*pos) >= mbox->max_union.max_value) {
        *pos = 0;
    }
}

static bool __sys_mbox_queue_enqueue(vsf_eda_queue_t *pthis, void *node)
{
    sys_mbox_t *mbox = (sys_mbox_t *)pthis;
    mbox->queue[mbox->tail] = node;
    __sys_mbox_next(mbox, &mbox->tail);
    return true;
}

static bool __sys_mbox_queue_dequeue(vsf_eda_queue_t *pthis, void **node)
{
    sys_mbox_t *mbox = (sys_mbox_t *)pthis;
    *node = mbox->queue[mbox->head];
    __sys_mbox_next(mbox, &mbox->head);
    return true;
}

static const vsf_eda_queue_op_t __sys_mbox_queue_op = {
    .enqueue = __sys_mbox_queue_enqueue,
    .dequeue = __sys_mbox_queue_dequeue,
};

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    VSF_ASSERT(size <= VSF_SYNC_MAX);

    // vsf_eda_queue_init can accept a minimal size of 1
    if (size <= 0) {
        size = 1;
    }

    mbox->queue = vsf_heap_malloc(sizeof(void *) * size);
    if (NULL == mbox->queue) {
        return ERR_MEM;
    }
    mbox->head = mbox->tail = 0;

    mbox->use_as__vsf_eda_queue_t.op = __sys_mbox_queue_op;
    vsf_eda_queue_init(&mbox->use_as__vsf_eda_queue_t, size);
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
//    vsf_eda_queue_fini(&mbox->use_as__vsf_eda_queue_t);
    if (mbox->queue != NULL) {
        vsf_heap_free(mbox->queue);
    }
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    return mbox->queue != NULL ? 1 : 0;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    mbox->queue = NULL;
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (VSF_ERR_NONE == vsf_eda_queue_send(&mbox->use_as__vsf_eda_queue_t, msg, 0)) {
        return ERR_OK;
    }
    return ERR_MEM;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    if (VSF_ERR_NONE != vsf_eda_queue_send(&mbox->use_as__vsf_eda_queue_t, msg, -1)) {
        vsf_sync_reason_t reason;

        do {
            reason = vsf_eda_queue_send_get_reason(&mbox->use_as__vsf_eda_queue_t, vsf_thread_wait(), msg);
        } while (reason == VSF_SYNC_PENDING);
    }
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
    // TODO:
    return sys_mbox_trypost(q, msg);
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    if (VSF_ERR_NONE == vsf_eda_queue_recv(&mbox->use_as__vsf_eda_queue_t, msg, 0)) {
        return 0;
    }
    return SYS_MBOX_EMPTY;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    vsf_timeout_tick_t timeout_tick = (0 == timeout) ? -1 : vsf_systimer_ms_to_tick(timeout);
    vsf_systimer_tick_t start = vsf_systimer_get_tick();
    u32_t duration;
    vsf_sync_reason_t reason;

    if (VSF_ERR_NONE != vsf_eda_queue_recv(&mbox->use_as__vsf_eda_queue_t, msg, timeout_tick)) {
        do {
            reason = vsf_eda_queue_recv_get_reason(&mbox->use_as__vsf_eda_queue_t, vsf_thread_wait(), msg);
        } while (reason == VSF_SYNC_PENDING);
    } else {
        reason = VSF_SYNC_GET;
    }

    if (reason != VSF_SYNC_GET) {
        duration = SYS_ARCH_TIMEOUT;
    } else {
        duration = vsf_systimer_tick_to_ms(vsf_systimer_get_tick() - start);
    }
    return duration;
}

void sys_init(void)
{
}
#endif      // NO_SYS

#endif      // VSF_USE_TCPIP && VSF_USE_LWIP
