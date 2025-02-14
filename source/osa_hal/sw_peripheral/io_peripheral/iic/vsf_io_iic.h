/*****************************************************************************
 *   Copyright(C)2009-2020 by VSF Team                                       *
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
#include "../io_periperhal_cfg.h"

#ifndef __OSA_HAL_DRIVER_IO_IIC_H__
#define __OSA_HAL_DRIVER_IO_IIC_H__

#if VSF_USE_IO_IIC == ENABLED

#include "hal/vsf_hal.h"
#include "kernel/vsf_kernel.h"

/*! \NOTE: Make sure #include "utilities/ooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file
 *!        included in this file
 */

#if     defined(__VSF_IO_IIC_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT__

#elif   defined(__VSF_IO_IIC_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"

/*============================ MACROS ========================================*/

#define VSF_IO_IIC_NO_START             (1 << 0)
#define VSF_IO_IIC_NO_STOP              (1 << 1)
#define VSF_IO_IIC_NO_ADDR              (1 << 2)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

vsf_class(vsf_io_iic_t) {
    public_member(
        struct {
            vsf_gpio_t *port;
            uint8_t scl_pin;
            uint8_t sda_pin;
        } io;
        struct {
            void (*handler)(vsf_io_iic_t *io_iic, int_fast32_t remain_size);
        } callback;
        uint8_t freq_khz;
        uint8_t byte_interval_us;
    )

    private_member(
        union {
            struct {
                uint8_t is_rx       : 1;
                uint8_t is_tx       : 1;
                uint8_t is_started  : 1;
                uint8_t is_addr     : 1;
                uint8_t sda_sample  : 1;
            };
            uint8_t __state;
        };
        uint8_t flags;
        uint8_t state;
        uint8_t cur_data;
        uint8_t *buffer;
        int32_t size;
        vsf_callback_timer_t cb_timer;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern vsf_err_t vsf_io_iic_init(vsf_io_iic_t *io_iic);
extern vsf_err_t vsf_io_iic_read(vsf_io_iic_t *io_iic, uint_fast8_t addr, uint8_t *buffer, uint_fast32_t size, uint_fast8_t flags);
extern vsf_err_t vsf_io_iic_write(vsf_io_iic_t *io_iic, uint_fast8_t addr, uint8_t *buffer, uint_fast32_t size, uint_fast8_t flags);

#if defined(__VSF_IO_IIC_CLASS_IMPLEMENT) || defined(__VSF_IO_IIC_CLASS_INHERIT)
extern vsf_err_t vsf_io_iic_transact(vsf_io_iic_t *io_iic, uint_fast8_t addr, uint8_t *buffer, uint_fast32_t size, uint_fast8_t flags);
#endif

#undef __VSF_IO_IIC_CLASS_IMPLEMENT
#undef __VSF_IO_IIC_CLASS_INHERIT
#endif
#endif

/* EOF */
