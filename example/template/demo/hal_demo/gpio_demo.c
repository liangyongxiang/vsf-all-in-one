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

#include "vsf.h"

#if APP_USE_HAL_DEMO == ENABLED && APP_USE_HAL_GPIO_DEMO == ENABLED && VSF_HAL_USE_GPIO == ENABLED

/*============================ MACROS ========================================*/

#ifndef APP_GPIO_DEMO_CFG_INPUT_TEST
#   define APP_GPIO_DEMO_CFG_INPUT_TEST                 ENABLED
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_TEST
#   define APP_GPIO_DEMO_CFG_OUTPUT_TEST                ENABLED
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_TEST
#   define APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_TEST         ENABLED
#endif

#ifndef APP_GPIO_DEMO_CFG_INPUT_GPIO
#   define APP_GPIO_DEMO_CFG_INPUT_GPIO                 (vsf_gpio_t *)&vsf_gpio0
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_GPIO
#   define APP_GPIO_DEMO_CFG_OUTPUT_GPIO                (vsf_gpio_t *)&vsf_gpio1
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_GPIO
#   define APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_GPIO         (vsf_gpio_t *)&vsf_gpio1
#endif

#ifndef APP_GPIO_DEMO_CFG_INPUT_PIN_MASK
#   define APP_GPIO_DEMO_CFG_INPUT_PIN_MASK             (1 << 10)
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_PIN_MASK
#   define APP_GPIO_DEMO_CFG_OUTPUT_PIN_MASK            (1 << 3)
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_PIN_MASK
#   define APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_PIN_MASK     (1 << 2)
#endif

#ifndef APP_GPIO_DEMO_CFG_INPUT_FEATURE
#   define APP_GPIO_DEMO_CFG_INPUT_FEATURE              (IO_PULL_UP)
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_FEATURE
#   define APP_GPIO_DEMO_CFG_OUTPUT_FEATURE             (IO_PULL_UP)
#endif

#ifndef APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_FEATURE
#   define APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_FEATURE      (IO_PULL_UP)
#endif

#ifndef APP_GPIO_DEMO_PRIO
#   define APP_GPIO_DEMO_PRIO                           vsf_prio_0
#endif

#ifndef APP_GPIO_DEMO_DELAY_MS
#   define APP_GPIO_DEMO_DELAY_MS                       500
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct app_gpio_demo_t {
    vsf_teda_t teda;
} app_gpio_demo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static app_gpio_demo_t __app_gpio_demo;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __gpio_demo_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
#if APP_GPIO_DEMO_CFG_INPUT_TEST == ENABLED
    vsf_gpio_t *input_gpio = APP_GPIO_DEMO_CFG_INPUT_GPIO;
    uint32_t input_pin_mask = APP_GPIO_DEMO_CFG_INPUT_PIN_MASK;
    uint_fast32_t input_feature = APP_GPIO_DEMO_CFG_INPUT_FEATURE;
    uint32_t read_pins = 0;
#endif
#if APP_GPIO_DEMO_CFG_OUTPUT_TEST == ENABLED
    vsf_gpio_t *output_gpio = APP_GPIO_DEMO_CFG_OUTPUT_GPIO;
    uint32_t output_pin_mask = APP_GPIO_DEMO_CFG_OUTPUT_PIN_MASK;
    uint_fast32_t output_feature = APP_GPIO_DEMO_CFG_OUTPUT_FEATURE;
    static bool state = true;
#endif
#if APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_TEST == ENABLED
    vsf_gpio_t *output_toggle_gpio = APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_GPIO;
    uint32_t output_toggle_pin_mask = APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_PIN_MASK;
    uint_fast32_t output_toggle_feature = APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_FEATURE;
#endif

    switch (evt) {
    case VSF_EVT_INIT:
#if APP_GPIO_DEMO_CFG_INPUT_TEST == ENABLED
        vsf_gpio_config_pin(input_gpio, input_pin_mask, input_feature);
        vsf_gpio_set_input(input_gpio, input_pin_mask);
#endif
#if APP_GPIO_DEMO_CFG_OUTPUT_TEST == ENABLED
        vsf_gpio_config_pin(output_gpio, output_pin_mask, output_feature);
        vsf_gpio_set_output(output_gpio, output_pin_mask);
#endif
#if APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_TEST == ENABLED
        vsf_gpio_config_pin(output_toggle_gpio, output_toggle_pin_mask, output_toggle_feature);
        vsf_gpio_set_output(output_toggle_gpio, output_toggle_pin_mask);
#endif

    case VSF_EVT_TIMER:
#if APP_GPIO_DEMO_CFG_INPUT_TEST == ENABLED
        read_pins  = vsf_gpio_read(input_gpio);
        read_pins &= input_pin_mask;
        vsf_trace_debug("read pin value: 0x%08x" VSF_TRACE_CFG_LINEEND, read_pins);
#endif

#if APP_GPIO_DEMO_CFG_OUTPUT_TEST == ENABLED
        if (state) {
            vsf_gpio_set(output_gpio, APP_GPIO_DEMO_CFG_OUTPUT_PIN_MASK);
        } else {
            vsf_gpio_clear(output_gpio, APP_GPIO_DEMO_CFG_OUTPUT_PIN_MASK);
        }
        state = !state;
#endif

#if APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_TEST == ENABLED
        vsf_gpio_toggle(output_toggle_gpio, APP_GPIO_DEMO_CFG_OUTPUT_TOGGLE_PIN_MASK);
#endif
        vsf_teda_set_timer_ms(APP_GPIO_DEMO_DELAY_MS);
        break;
    }
}

#if APP_USE_LINUX_DEMO == ENABLED
int gpio_main(int argc, char *argv[])
{
#else
int VSF_USER_ENTRY(void)
{
#   if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#   endif
#   if USRAPP_CFG_STDIO_EN == ENABLED
    vsf_stdio_init();
#   endif
#endif

    const vsf_eda_cfg_t cfg = {
        .fn.evthandler  = __gpio_demo_evthandler,
        .priority       = APP_GPIO_DEMO_PRIO,
    };
    return vsf_teda_start(&__app_gpio_demo.teda, (vsf_eda_cfg_t*)&cfg);
}

#endif
