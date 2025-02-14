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

#include "../driver.h"

#if VSF_HAL_USE_USART == ENABLED

#include "../vendor/plf/aic8800/src/driver/uart/reg_uart1.h"
#include "../vendor/plf/aic8800/src/driver/iomux/reg_iomux.h"
#include "../vendor/plf/aic8800/src/driver/ipc/reg_ipc_comreg.h"
#include "sysctrl_api.h"
//#include "reg_access.h"

#define VSF_USART_CFG_IMPLEMENT_OP                      ENABLED
#define VSF_USART_CFG_IMPLEMENT_REQUEST_BY_FIFO         ENABLED
#define VSF_USART_CFG_INSTANCE_PREFIX                   vsf_hw
#include "hal/driver/common/usart/usart_template.inc"
#include "hal/driver/common/usart/usart_request.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define __VSF_HW_USART_IMP_LV0(__count, __dont_care)                            \
    static const vsf_hw_usart_clock_t vsf_usart##__count##_clock = {            \
        .hclk   = CSC_HCLKME_UART##__count##_EN_BIT,                            \
        .oclk   = CSC_OCLKME_UART##__count##_EN_BIT,                            \
        .perclk = PER_UART##__count,                                            \
    };                                                                          \
    static vsf_hw_usart_t __vsf_usart##__count = {                              \
        VSF_USART_OP                                                            \
        .reg    = UART##__count,                                                \
        .irqn   = UART##__count##_IRQn,                                         \
        .clock = &vsf_usart##__count##_clock,                                   \
    };                                                                          \
    vsf_usart_request_t vsf_usart##__count = {                                  \
        VSF_USART_FIFO2REQ_OP                                                   \
        .real_usart_ptr = (vsf_usart_t *)&__vsf_usart##__count,                 \
    };                                                                          \
    void UART##__count##_IRQHandler(void)                                       \
    {                                                                           \
        __vsf_hw_usart_irq_handler(&__vsf_usart##__count);                      \
    }

/*============================ TYPES =========================================*/

typedef struct vsf_hw_usart_clock_t {
    uint32_t hclk;
    uint32_t oclk;
    uint32_t perclk;
} vsf_hw_usart_clock_t;

typedef struct vsf_hw_usart_t {
#if VSF_USART_CFG_MULTI_INSTANCES == ENABLED
    vsf_usart_t vsf_usart;
#endif

    vsf_usart_isr_t     isr;

    const vsf_hw_usart_clock_t *clock;

    IRQn_Type irqn;
    uart_reg_t *reg;

    usart_status_t       status;
    em_usart_irq_mask_t  irq_mask;
    uart_reg_t           reg_dump[10];
    uint8_t              index;
} vsf_hw_usart_t;


/*============================ PROTOTYPES ====================================*/

static void __vsf_hw_usart_irq_handler(vsf_hw_usart_t *usart_ptr);

/*============================ GLOBAL VARIABLES ==============================*/

#if USART_MASK & (1 << 0)
__VSF_HW_USART_IMP_LV0(0, NULL)
#endif
#if USART_MASK & (1 << 1)
__VSF_HW_USART_IMP_LV0(1, NULL)
#endif
#if USART_MASK & (1 << 2)
__VSF_HW_USART_IMP_LV0(2, NULL)
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ IMPLEMENTATION ================================*/

vsf_err_t vsf_hw_usart_init(vsf_usart_t *usart_ptr, usart_cfg_t *cfg_ptr)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);
    const vsf_hw_usart_clock_t *clock = hw_usart_ptr->clock;

    cpusysctrl_hclkme_set(clock->hclk);                                     // clock enable
    cpusysctrl_oclkme_set(clock->oclk);

    reg->DBUFCFG_REG = 0x0;                                                 // reset fifo
    reg->IRQCTL_REG = 0x0;                                                  // clean all interrupt
    reg->DBUFTH_REG = (1 << UART_RXTRIGTH) |  (0 << UART_TXTRIGTH);         // tx and rx data buffer trigger threshold
    reg->MDMCFG_REG |= UART_CLK_P_MSK;                                      // force 48M Clock
    reg->DFMTCFG_REG = cfg_ptr->mode & __USART_AIC8800_MASK;

    uint32_t div = sysctrl_clock_get(clock->perclk) / cfg_ptr->baudrate;
    reg->DFMTCFG_REG |= UART_DIVAE_MSK;                                     // div reg access enable
    reg->DIV0_REG = (div >> 4) & UART_DIV0_MSK;
    reg->DIV1_REG = (div >> 12) & UART_DIV1_MSK;
    reg->DIV2_REG = (div & 0x1) + ((div >> 1) & 0x7) + ((div << 3) & 0x7);
    reg->DFMTCFG_REG &= ~UART_DIVAE_MSK;                                    // div reg access disable

    reg->DBUFCFG_REG = UART_DBUFEN_MSK | UART_RXDRST_MSK | UART_TXDRST_MSK; // tx/rx data buf reset and enable

    vsf_usart_isr_t *isr_ptr = &cfg_ptr->isr;
    hw_usart_ptr->isr = *isr_ptr;

    if (isr_ptr->handler_fn != NULL) {
        NVIC_SetPriority(hw_usart_ptr->irqn, (uint32_t)isr_ptr->prio);
        NVIC_EnableIRQ(hw_usart_ptr->irqn);
    } else {
        NVIC_DisableIRQ(hw_usart_ptr->irqn);
    }

    return VSF_ERR_NONE;
}

fsm_rt_t vsf_hw_usart_enable(vsf_usart_t *usart_ptr)
{
    return fsm_rt_cpl;
}

fsm_rt_t vsf_hw_usart_disable(vsf_usart_t *usart_ptr)
{
    return fsm_rt_cpl;
}

void vsf_hw_usart_irq_enable(vsf_usart_t *usart_ptr, em_usart_irq_mask_t irq_mask)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    VSF_HAL_ASSERT(NULL != hw_usart_ptr->reg);
    VSF_HAL_ASSERT((irq_mask & ~USART_IRQ_MASK_FIFO) == 0);

    hw_usart_ptr->reg->IRQCTL_REG |= irq_mask;
}

void vsf_hw_usart_irq_disable(vsf_usart_t *usart_ptr, em_usart_irq_mask_t irq_mask)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    VSF_HAL_ASSERT(NULL != hw_usart_ptr->reg);
    VSF_HAL_ASSERT((irq_mask & ~USART_IRQ_MASK_FIFO) == 0);

    hw_usart_ptr->reg->IRQCTL_REG &= ~irq_mask;
}

usart_status_t vsf_hw_usart_status(vsf_usart_t *usart_ptr)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);

    return hw_usart_ptr->status;
}

static bool __hw_usart_read_fifo_is_empty(vsf_hw_usart_t *hw_usart_ptr)
{
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);

    return reg->DBUFSTS_REG & UART_RX_DBUF_EMPTY_MSK;
}

static bool __hw_usart_write_fifo_is_full(vsf_hw_usart_t *hw_usart_ptr)
{
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);

    return reg->DBUFSTS_REG & UART_TX_DBUF_FULL_MSK;
}

uint_fast16_t vsf_hw_usart_fifo_read(vsf_usart_t *usart_ptr, void *buffer_ptr, uint_fast16_t count)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);

    uint8_t *buf_ptr = (uint8_t *) buffer_ptr;
    VSF_HAL_ASSERT(NULL != buf_ptr);

    uint_fast16_t i = 0;
    while (i < count) {
        if (__hw_usart_read_fifo_is_empty(hw_usart_ptr)) {
            break;
        }
        buf_ptr[i++] = reg->TXRXD_REG & UART_TXDATA_MSK;
    }
    return i;
}

uint_fast16_t vsf_hw_usart_fifo_write(vsf_usart_t *usart_ptr, void *buffer_ptr, uint_fast16_t count)
{
    vsf_hw_usart_t *hw_usart_ptr = (vsf_hw_usart_t *)usart_ptr;
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);

    uint8_t *buf_ptr = (uint8_t *) buffer_ptr;
    VSF_HAL_ASSERT(NULL != buf_ptr);

    uint_fast16_t i = 0;
    while (i < count) {
        if (__hw_usart_write_fifo_is_full(hw_usart_ptr)) {
            break;
        }
        reg->TXRXD_REG = buf_ptr[i++] & UART_TXDATA_MSK;
    }
    return i;
}

static em_usart_irq_mask_t __get_uart_irq_mask(vsf_hw_usart_t *hw_usart_ptr)
{
    VSF_HAL_ASSERT(NULL != hw_usart_ptr);
    uart_reg_t *reg = hw_usart_ptr->reg;
    VSF_HAL_ASSERT(NULL != reg);

    uint32_t value;
    uint32_t irq_type = reg->IRQTYP_REG & UART_IRQTYP_MSK;

    switch (irq_type) {
        case UART_IRQTYP_RX_ERROR_INT:
            value = reg->IRQSTS_REG;        // TODO: report more specific rx error
            (void)value;
            return USART_IRQ_MASK_RX_ERROR;

        case UART_IRQTYP_RX_INT:
            return USART_IRQ_MASK_RX;

        case UART_IRQTYP_TIMEOUT_INT:
            return 0;

        case UART_IRQTYP_TX_INT:
            return USART_IRQ_MASK_TX;

        case UART_IRQTYP_MODEM_INT:
            value = reg->MDMSTS_REG;        // TODO: report more specific moden line interrupt
            (void)value;
            return (em_usart_irq_mask_t)0;

        case UART_IRQTYP_NO_INT:
            return (em_usart_irq_mask_t)0;

        default:
            VSF_HAL_ASSERT(0);
            return (em_usart_irq_mask_t)0;
    }
}

static void __vsf_hw_usart_irq_handler(vsf_hw_usart_t *hw_usart_ptr)
{
    em_usart_irq_mask_t irq_mask = __get_uart_irq_mask(hw_usart_ptr);
    if (irq_mask & USART_IRQ_MASK) {
        if (NULL != hw_usart_ptr->isr.handler_fn) {
            hw_usart_ptr->isr.handler_fn(hw_usart_ptr->isr.target_ptr,
                                         (vsf_usart_t *)hw_usart_ptr, irq_mask);
        }
    }
}


#endif      // VSF_HAL_USE_USART
