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

#ifndef __VSF_HEAP_H__
#define __VSF_HEAP_H__

/*============================ INCLUDES ======================================*/

#include "service/vsf_service_cfg.h"
#if VSF_USE_HEAP == ENABLED

#include "utilities/vsf_utilities.h"

#if VSF_HEAP_CFG_TRACE == ENABLED
#   include "service/trace/vsf_trace.h"
#endif

/*! \NOTE: Make sure #include "utilities/ooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file
 *!        included in this file
 */

#if     defined(__VSF_HEAP_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT__
#   undef __VSF_HEAP_CLASS_IMPLEMENT
#elif   defined(__VSF_HEAP_CLASS_INHERIT__)
#   define __PLOOC_CLASS_INHERIT__
#   undef __VSF_HEAP_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

#ifndef VSF_HEAP_SIZE
#   warning VSF_HEAP_SIZE is not defined, use 128K as default
#   define VSF_HEAP_SIZE    (128 * 1024)
#endif

#if VSF_HEAP_CFG_TRACE == ENABLED
#   define vsf_heap_malloc_aligned(...)                                         \
    ({                                                                          \
        void * ptr = vsf_heap_malloc_aligned_imp(__VA_ARGS__);                  \
        vsf_trace_debug("%s: malloc_align 0x%p" VSF_TRACE_CFG_LINEEND, __FUNCTION__, ptr);\
        ptr;                                                                    \
    })
#   define vsf_heap_malloc(...)                                                 \
    ({                                                                          \
        void * ptr = vsf_heap_malloc_imp(__VA_ARGS__);                          \
        vsf_trace_debug("%s: malloc 0x%p" VSF_TRACE_CFG_LINEEND, __FUNCTION__, ptr);\
        ptr;                                                                    \
    })
#   define vsf_heap_realloc_aligned(...)                                        \
    ({                                                                          \
        void * ptr = vsf_heap_realloc_aligned_imp(__VA_ARGS__);                 \
        vsf_trace_debug("%s: realloc_align 0x%p" VSF_TRACE_CFG_LINEEND, __FUNCTION__, ptr);\
        ptr;                                                                    \
    })
#   define vsf_heap_realloc(...)                                                \
    ({                                                                          \
        void * ptr = vsf_heap_realloc_imp(__VA_ARGS__);                         \
        vsf_trace_debug("%s: realloc 0x%p" VSF_TRACE_CFG_LINEEND, __FUNCTION__, ptr);\
        ptr;                                                                    \
    })
#   define vsf_heap_free(__ptr)                                                 \
    ({                                                                          \
        vsf_trace_debug("%s: free 0x%p" VSF_TRACE_CFG_LINEEND, __FUNCTION__, (__ptr));\
        vsf_heap_free_imp(__ptr);                                               \
    })
#else
#   define vsf_heap_malloc_aligned          vsf_heap_malloc_aligned_imp
#   define vsf_heap_malloc                  vsf_heap_malloc_imp
#   define vsf_heap_realloc_aligned         vsf_heap_realloc_aligned_imp
#   define vsf_heap_realloc                 vsf_heap_realloc_imp
#   define vsf_heap_free                    vsf_heap_free_imp
#endif

/*============================ TYPES =========================================*/

vsf_class(vsf_heap_t) {
    protected_member(
        vsf_dlist_t * (*get_freelist)(uint_fast32_t size);
    )
};

declare_interface(i_heap_t)

//! \name vsf heap interface
//! @{
def_interface(i_heap_t)
    void (*Init)            (void);
    void (*AddBuffer)       (uint8_t *buffer, uint_fast32_t size);
    void (*AddMemory)       (vsf_mem_t mem);
    void *(*MallocAligned)  (uint_fast32_t size, uint_fast32_t alignment);
    void *(*Malloc)         (uint_fast32_t size);
    void *(*ReallocAligned) (void *buffer, uint_fast32_t size, uint_fast32_t alignment);
    void *(*Realloc)        (void *buffer, uint_fast32_t size);
    void (*Free)            (void *buffer);
end_def_interface(i_heap_t)
//! @}

/*============================ GLOBAL VARIABLES ==============================*/

extern const i_heap_t VSF_HEAP;

/*============================ PROTOTYPES ====================================*/

// heap class
extern void __vsf_heap_add_buffer(vsf_heap_t *heap, uint8_t *buffer, uint_fast32_t size);
extern void * __vsf_heap_malloc_aligned(vsf_heap_t *heap, uint_fast32_t size, uint_fast32_t alignment);
extern void * __vsf_heap_realloc_aligned(vsf_heap_t *heap, void *buffer, uint_fast32_t size, uint_fast32_t alignment);
extern void __vsf_heap_free(vsf_heap_t *heap, void *buffer);

// default heap
extern void vsf_heap_init(void);
/*!\note: vsf_heap_add_buffer and vsf_heap_add_memory cannot add misaligned memory spaces.
 *!\when a user needs to add memory space, the space provided each time must be aligned
 *!\according to the alignment defined by VSF_HEAP_CFG_MCB_ALIGN_BIT(2 if undefined).
*/
extern void vsf_heap_add_buffer(uint8_t *buffer, uint_fast32_t size);
extern void vsf_heap_add_memory(vsf_mem_t mem);
extern void * vsf_heap_malloc_aligned_imp(uint_fast32_t size, uint_fast32_t alignment);
extern void * vsf_heap_malloc_imp(uint_fast32_t size);
extern void * vsf_heap_realloc_aligned_imp(void *buffer, uint_fast32_t size, uint_fast32_t alignment);
extern void * vsf_heap_realloc_imp(void *buffer, uint_fast32_t size);
extern void vsf_heap_free_imp(void *buffer);

#endif

#ifdef __cplusplus
}
#endif

#endif
