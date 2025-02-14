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

/****************************************************************************
*  Copyright 2020 by Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)    *
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
#include "vsf.h"

#if VSF_USE_TINY_GUI == ENABLED
#include <stdio.h>
#include "./images/demo_images.h"
#if APP_USE_TGUI_DESIGNER_DEMO == ENABLED
#   include "./tgui_designer/tgui_designer.h"
#endif
#include "./stopwatch/stopwatch.h"
/*============================ MACROS ========================================*/
#define DEMO_OFFSET            0

#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP | VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP | VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM | VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM | VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_CENTER)

#ifndef APP_TGUI_DEMO_CFG_QUEUE_MAX
#   if APP_USE_TGUI_DESIGNER_DEMO == ENABLED
#       define APP_TGUI_DEMO_CFG_QUEUE_MAX      320
#   else
#       define APP_TGUI_DEMO_CFG_QUEUE_MAX      32
#endif
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/
NO_INIT vsf_tgui_t g_tTGUIDemo;

#if APP_USE_TGUI_DESIGNER_DEMO == ENABLED
static NO_INIT tgui_designer_t s_tDesigner;
#else
NO_INIT union {
    stopwatch_t stopwatch;
    popup_t popup;
} panels;
#endif
/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_tgui_low_level_on_ready_to_refresh(void)
{
    vsf_tgui_low_level_refresh_ready(&g_tTGUIDemo);
}

vsf_err_t tgui_demo_init(void)
{
    NO_INIT static vsf_tgui_evt_t s_tEvtQueueBuffer[APP_TGUI_DEMO_CFG_QUEUE_MAX];

#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
    NO_INIT static uint16_t s_tBFSBuffer[APP_TGUI_DEMO_CFG_QUEUE_MAX];
#endif

    const vsf_tgui_cfg_t cfg = {
        .evt_queue = {
            .obj_ptr = s_tEvtQueueBuffer,
            .s32_size = sizeof(s_tEvtQueueBuffer)
        },
#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
        .bfs_queue = {
            .obj_ptr = s_tBFSBuffer,
            .s32_size = sizeof(s_tBFSBuffer),
        },
#endif
    };

    vsf_err_t err = vk_tgui_init(&g_tTGUIDemo, &cfg);

#if APP_USE_TGUI_DESIGNER_DEMO == ENABLED
    tgui_designer_init(&s_tDesigner, &s_tTGUIDemo);
    vk_tgui_set_root_container(&s_tTGUIDemo, (vsf_tgui_root_container_t *)&s_tDesigner, true);
#else
    //my_stopwatch_init(&g_tMyStopwatch, &g_tTGUIDemo);
    popup_init(&panels.popup);

    vk_tgui_set_root_container(&g_tTGUIDemo, (vsf_tgui_root_container_t *)&panels.popup, true);
#endif

    return err;
}

void vsf_tgui_on_keyboard_evt(vk_keyboard_evt_t* evt)
{
//! this block of code is used for test purpose only
    vsf_tgui_evt_t event = {
        .tKeyEvt = {
            .msg = vsf_input_keyboard_is_down(evt)
                                ? VSF_TGUI_EVT_KEY_DOWN
                                : VSF_TGUI_EVT_KEY_UP,
            .hwKeyValue = vsf_input_keyboard_get_keycode(evt),
        },
    };

    vk_tgui_send_message(&g_tTGUIDemo, event);

    if (!vsf_input_keyboard_is_down(evt)) {
        event.tKeyEvt.msg = VSF_TGUI_EVT_KEY_PRESSED;
        vk_tgui_send_message(&g_tTGUIDemo, event);
    }
}


void vsf_tgui_on_touchscreen_evt(vk_touchscreen_evt_t* ts_evt)
{
    //! this block of code is used for test purpose only
    vsf_err_t result =
        vsf_tgui_send_touch_evt(&g_tTGUIDemo,
                                0,  /* only one finger is used for now*/
                                (vsf_tgui_location_t) {
                                    vsf_input_touchscreen_get_x(ts_evt),
                                    vsf_input_touchscreen_get_y(ts_evt)},
                                vsf_input_touchscreen_is_down(ts_evt),
                                ts_evt->use_as__vk_input_evt_t.duration
                                );

    VSF_ASSERT(result == VSF_ERR_NONE);
}

#if (VSF_TGUI_CFG_SUPPORT_MOUSE_LIKE_EVENTS == ENABLED) && defined(VSF_TGUI_CFG_SUPPORT_MOUSE_LIKE_EVENTS)
void vsf_tgui_on_mouse_evt(vk_mouse_evt_t *mouse_evt)
{
/*
    switch (vk_input_mouse_evt_get(mouse_evt)) {
        case VSF_INPUT_MOUSE_EVT_BUTTON:
            vsf_trace_debug("mouse button: %d %s @(%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_button_get(mouse_evt),
                vk_input_mouse_evt_button_is_down(mouse_evt) ? "down" : "up",
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
        case VSF_INPUT_MOUSE_EVT_MOVE:
            vsf_trace_debug("mouse move: @(%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
        case VSF_INPUT_MOUSE_EVT_WHEEL:
            vsf_trace_debug("mouse wheel: (%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
    }

*/
    //! this block of code is used for test purpose only
    int_fast8_t button_id = vk_input_mouse_evt_button_get(mouse_evt) == 0 ? 0 : 1;
    static uint_fast8_t __button_status = 0;
    vsf_err_t result = VSF_ERR_NONE;

    switch (vk_input_mouse_evt_get(mouse_evt)) {
        case VSF_INPUT_MOUSE_EVT_BUTTON: {

            result =
                vsf_tgui_send_touch_evt(&g_tTGUIDemo,
                                        button_id,
                                        (vsf_tgui_location_t) {
                                            vk_input_mouse_evt_get_x(mouse_evt),
                                            vk_input_mouse_evt_get_y(mouse_evt)},
                                        vk_input_mouse_evt_button_is_down(mouse_evt),
                                        mouse_evt->use_as__vk_input_evt_t.duration
                                        );

            if (vk_input_mouse_evt_button_is_down(mouse_evt)) {
                __button_status |= BIT(button_id);
            } else {
                __button_status &= ~BIT(button_id);
            }


            VSF_ASSERT(result == VSF_ERR_NONE);
            break;
        }

        case VSF_INPUT_MOUSE_EVT_MOVE: {
            if (0 == __button_status) {
                //! pure mouse move
                result =
                    vsf_tgui_send_touch_evt(&g_tTGUIDemo,
                                            button_id,
                                            (vsf_tgui_location_t) {
                                                vk_input_mouse_evt_get_x(mouse_evt),
                                                vk_input_mouse_evt_get_y(mouse_evt)},
                                            __button_status & BIT(button_id),
                                            mouse_evt->use_as__vk_input_evt_t.duration
                                            );
            } else {
                if (__button_status & BIT(0)) {
                    //! simulate finger 0 move
                    result =
                    vsf_tgui_send_touch_evt(&g_tTGUIDemo,
                                            0,
                                            (vsf_tgui_location_t) {
                                                vk_input_mouse_evt_get_x(mouse_evt),
                                                vk_input_mouse_evt_get_y(mouse_evt)},
                                            __button_status & BIT(0),
                                            mouse_evt->use_as__vk_input_evt_t.duration
                                            );
                }
                if (__button_status & BIT(1)) {
                    //! simulate finger 1 move
                    result =
                    vsf_tgui_send_touch_evt(&g_tTGUIDemo,
                                            1,
                                            (vsf_tgui_location_t) {
                                                vk_input_mouse_evt_get_x(mouse_evt),
                                                vk_input_mouse_evt_get_y(mouse_evt)},
                                            __button_status & BIT(1),
                                            mouse_evt->use_as__vk_input_evt_t.duration
                                            );
                }
            }
            VSF_ASSERT(result == VSF_ERR_NONE);
            break;
        }


        case VSF_INPUT_MOUSE_EVT_WHEEL:  {
                vsf_tgui_evt_t event = {
                    .tGestureEvt = {
                        .msg = VSF_TGUI_EVT_GESTURE_WHEEL,
                        .delta = {
                            .iX = 0,
                            .iY = -vk_input_mouse_evt_get_y(mouse_evt),
                            .ms = 20,            //! 50Hz
                        },
                    },
                };

                vsf_tgui_control_set_active(vsf_tgui_pointed_control_get(&g_tTGUIDemo));
                vk_tgui_send_message(&g_tTGUIDemo, event);
                break;
            }

        default:
            break;
    }

}
#endif

#endif


/* EOF */
