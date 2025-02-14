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

#if APP_USE_NNOM_DEMO == ENABLED

#include "../common/usrapp_common.h"
#include <stdio.h>

#include "nnom.h"

#include "component/3rd-party/nnom/raw/examples/mnist-simple/mcu/image.h"
#include "component/3rd-party/nnom/raw/examples/mnist-simple/mcu/weights.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct ai_demo_t {
    nnom_model_t *model;
    union {
        nnom_mnist_model_data_t mnist_data;
    };
};
typedef struct ai_demo_t ai_demo_t;

/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static ai_demo_t ai_demo;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

const char codeLib[] = "@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.   ";
void print_img(int8_t * buf)
{
    for(int y = 0; y < 28; y++)  {
        for (int x = 0; x < 28; x++)  {
            int index = 69 / 127.0 * (127 - buf[y*28+x]);
            if(index > 69) index = 69;
            if(index < 0) index = 0;
            printf("%c",codeLib[index]);
            printf("%c",codeLib[index]);
        }
        printf("\n");
    }
}

void ai_demo_mnist(uint_fast8_t index)
{
    vsf_systimer_tick_t tick, time;
    uint32_t predic_label;
    float prob;

    printf("\nprediction start.. \n");
    tick = vsf_systimer_get_tick();

    // copy data and do prediction
    memcpy(ai_demo.mnist_data.input_data, (int8_t*)&img[index][0], 784);
    nnom_predict(ai_demo.model, &predic_label, &prob);
    time = vsf_systimer_get_tick() - tick;

    //print original image to console
    print_img((int8_t*)&img[index][0]);

    printf("Time: %d ms\n", vsf_systimer_tick_to_ms(time));
    printf("Truth label: %d\n", label[index]);
    printf("Predicted label: %d\n", predic_label);
    printf("Probability: %d%%\n", (int)(prob*100));

    model_stat(ai_demo.model);
	printf("Total Memory cost (Network and NNoM): %d\n", nnom_mem_stat());
}

#if APP_USE_LINUX_DEMO == ENABLED
int nnom_main(int argc, char *argv[])
{
#else
int VSF_USER_ENTRY(void)
{
#   if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#       if USRAPP_CFG_STDIO_EN == ENABLED
    vsf_stdio_init();
#       endif
#   endif
#endif
    ai_demo.model = nnom_mnist_model_create(&ai_demo.mnist_data);
    model_run(ai_demo.model);

    for (int i = 0; i < dimof(label); i++) {
        ai_demo_mnist(i);
    }

    model_delete(ai_demo.model);
    ai_demo.model = NULL;
    return 0;
}

#endif      // APP_USE_NNOM_DEMO

/* EOF */
