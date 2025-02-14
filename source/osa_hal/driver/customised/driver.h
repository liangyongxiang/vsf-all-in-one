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

#ifndef __VSF_OSA_CUSTOMISED_HAL_DRIVER_H__
#define __VSF_OSA_CUSTOMISED_HAL_DRIVER_H__

/*============================ INCLUDES ======================================*/
#include "osa_hal/vsf_osa_hal_cfg.h"

#if VSF_USE_MULTIPLEX_HAL == ENABLED
#   include "./multiplex_hal/driver.h"
#endif

#if VSF_USE_STREAM_HAL == ENABLED
#   include "./stream_hal/driver.h"
#endif

#if     VSF_USE_LV2_HAL_XXXXXA == ENABLED
#   include "./XXXXXA/driver.h"

#elif   VSF_USE_LV2_HAL_XXXXXB == ENABLED
#   include "./XXXXXB/driver.h"
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/


#endif
/* EOF */
