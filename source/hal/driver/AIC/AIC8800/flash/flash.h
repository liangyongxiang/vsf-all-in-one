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

#ifndef __HAL_DRIVER_AIC8800_FLASH_H__
#define __HAL_DRIVER_AIC8800_FLASH_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#if VSF_HAL_USE_FLASH == ENABLED
#include "../__device.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct aic8800_flash_info_t {
    uint32_t                flash_size;
} aic8800_flash_info_t;

#include "hal/driver/common/template/vsf_template_flash.h"

/*============================ TYPES =========================================*/

struct vsf_flash_t {
    flash_cfg_t             cfg;
    aic8800_flash_info_t    info;
    bool                    is_enabled;
};

/*============================ INCLUDES ======================================*/
/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_flash_t vsf_flash0;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#endif /* VSF_HAL_USE_FLASH */
#endif /* EOF */