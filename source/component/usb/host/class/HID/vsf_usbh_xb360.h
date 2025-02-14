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

#ifndef __VSF_USBH_XB360_H__
#define __VSF_USBH_XB360_H__


/*============================ INCLUDES ======================================*/
#include "component/usb/vsf_usb_cfg.h"

#if VSF_USE_USB_HOST == ENABLED && VSF_USBH_USE_XB360 == ENABLED

#include "component/usb/common/class/HID/vsf_usb_xb360.h"
#if VSF_USE_INPUT == ENABLED && VSF_INPUT_USE_XB360 == ENABLED
#   include "component/input/vsf_input.h"
#endif

#if     defined(__VSF_USBH_XB360_CLASS_IMPLEMENT)
#   undef __VSF_USBH_XB360_CLASS_IMPLEMENT
#   define __PLOOC_CLASS_IMPLEMENT__
#endif
#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

#if VSF_USBH_USE_HID != ENABLED
#   error "XB360 driver need VSF_USBH_USE_HID"
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if VSF_USE_INPUT == ENABLED && VSF_INPUT_USE_XB360 == ENABLED
enum {
    VSF_INPUT_TYPE_XB360 = VSF_INPUT_USER_TYPE,
};

typedef struct vk_input_xb360_t {
    vsf_usb_xb360_gamepad_in_report_t data;
    vk_input_timestamp_t timestamp;
} vk_input_xb360_t;
#endif

// xb360 controller is not HID class, but almost compatible with HID class
vsf_class(vk_usbh_xb360_t) {

    implement(vk_usbh_hid_teda_t)
#if VSF_USE_INPUT == ENABLED && VSF_INPUT_USE_XB360 == ENABLED
    implement(vk_input_xb360_t)
#endif

    private_member(
        // gamepad_out_buf should be aligned
        vsf_usb_xb360_gamepad_out_report_t gamepad_out_buf;
        bool out_idle;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_INPUT == ENABLED && VSF_INPUT_USE_XB360 == ENABLED
extern const vk_input_item_info_t vk_xb360_gamepad_item_info[GAMEPAD_ID_NUM];
#endif

extern const vk_usbh_class_drv_t vk_usbh_xb360_drv;

/*============================ PROTOTYPES ====================================*/

#if VSF_USE_INPUT == ENABLED && VSF_INPUT_USE_XB360 == ENABLED
extern void vk_xb360_process_input(vk_input_xb360_t *dev, vsf_usb_xb360_gamepad_in_report_t *data);
extern void vk_xb360_new_dev(vk_input_xb360_t *dev);
extern void vk_xb360_free_dev(vk_input_xb360_t *dev);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif
