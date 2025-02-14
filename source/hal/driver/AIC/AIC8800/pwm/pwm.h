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
 *  limitations under the License.                                           *x
 *                                                                           *
 ****************************************************************************/

#ifndef __HAL_DRIVER_AIC8800_PWM_H__
#define __HAL_DRIVER_AIC8800_PWM_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#if VSF_HAL_USE_PWM == ENABLED
#include "../__device.h"
#include "./i_reg_pwm.h"

/*============================ MACROS ========================================*/

#ifndef VSF_HAL_PWM_IMP_TIME_SETTING
#   define VSF_HAL_PWM_IMP_TIME_SETTING                 ENABLED
#endif
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ INCLUDES ======================================*/

#include "hal/driver/common/template/vsf_template_pwm.h"

/*============================ TYPES =========================================*/

struct vsf_pwm_t {
    PWM_REG_T           *PARAM;
    pwm_cfg_t           cfg;
    bool                is_enabled;
};

/*============================ INCLUDES ======================================*/
/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_pwm_t vsf_pwm0;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#endif /* VSF_HAL_USE_PWM */
#endif /* EOF */