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

#ifndef __HAL_DRIVER_SPI_INTERFACE_H__
#define __HAL_DRIVER_SPI_INTERFACE_H__

/*============================ INCLUDES ======================================*/

#include "./vsf_template_hal_driver.h"
#include "hal/arch/vsf_arch.h"

/*============================ MACROS ========================================*/

#define SPI_DATASIZE(__N)       VSF_MCONNECT2(SPI_MODE_DATASIZE_, __N)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef enum em_spi_mode_t em_spi_mode_t;
/*
//! \name spi working mode
//! @{
enum em_spi_mode_t {
    SPI_MASTER                  = 0x00,             //!< select master mode
    SPI_SLAVE                   = BIT(0),           //!< select slave mode

    SPI_MSB_FIRST               = 0x00,             //!< default enable MSB
    SPI_LSB_FIRST               = BIT(7)            //!< transfer LSB first

    SPI_CPOL_HIGH               = BIT(3),           //!< SCK clock polarity is high
    SPI_CPOL_LOW                = 0x00,             //!< SCK clock polarity is low
    SPI_CPHA_HIGH               = 0x00,             //!< SCK clock phase is high
    SPI_CPHA_LOW                = BIT(4),           //!< SCK clock phase is low

    SPI_MODE_SPI                = 0x00,             //!< the driver should at least support standard spi frame

    SPI_DATASIZE_8              = 0x7,              //!< datasize is 8 bits

    // device specific settings
    SPI_MODE_TI                 = BIT(1),
    SPI_MODE_MICROWIRE          = BIT(2),

    SPI_LOOP_BACK               = BIT(5),           //!< enable loop back
};
//! @}
*/

/*! \brief em_spi_irq_mask_t
 *! \note em_spi_irq_mask_t should provide irq masks
//! @{
enum em_spi_irq_mask_t {
    // TX/RX reach fifo threshold, threshold on some devices is bound to 1
    SPI_IRQ_MASK_TX,
    SPI_IRQ_MASK_RX,

    // request_rx/request_tx complete
    SPI_IRQ_MASK_TX_CPL,
    SPI_IRQ_MASK_RX_CPL,

    // optional
    // FIFO
    SPI_IRQ_MASK_RX_FIFO_FULL,
    SPI_IRQ_MASK_TX_FIFO_EMPTY,
};
//! @}
 */

/* spi_status_t should implement peripheral_status_t */
typedef struct spi_status_t spi_status_t;

/* spi_capability_t should implement peripheral_capability_t */
typedef struct spi_capability_t spi_capability_t;

typedef enum em_spi_irq_mask_t em_spi_irq_mask_t;

typedef struct vsf_spi_t vsf_spi_t;

typedef void vsf_spi_isrhandler_t(  void *target_ptr,
                                    vsf_spi_t *spi_ptr,
                                    em_spi_irq_mask_t irq_mask);

//! \name spi isr for api
//! @{
typedef struct vsf_spi_isr_t {
    vsf_spi_isrhandler_t        *handler_fn;
    void                        *target_ptr;
    vsf_arch_prio_t             prio;
} vsf_spi_isr_t;

//! \name spi configuration for api
//! @{
typedef struct spi_cfg_t {
    uint32_t                    mode;               //!< spi working mode
    uint32_t                    clock_hz;
    vsf_spi_isr_t               isr;
} spi_cfg_t;
//! @}

dcl_interface(i_spi_t)

typedef void vsf_i_spi_isrhandler_t(void *target_ptr,
                                    const i_spi_t *i_spi_ptr,
                                    em_spi_irq_mask_t irq_mask);

//! \name spi isr for interface
//! @{
typedef struct vsf_i_spi_isr_t {
    vsf_i_spi_isrhandler_t      *handler_fn;
    void                        *target_ptr;
    vsf_arch_prio_t             prio;
} vsf_i_spi_isr_t;


//! \name spi configuration for interface
//! @{
typedef struct i_spi_cfg_t {
    uint32_t                    mode;               //!< spi working mode
    uint32_t                    clock_hz;
    vsf_i_spi_isr_t             isr;
} i_spi_cfg_t;
//! @}

//! \name class: spi_t
//! @{
def_interface(i_spi_t)
    union {
        implement(i_peripheral_t);
        struct {
            spi_status_t        (*Status)(void);
            spi_capability_t    (*Capability)(void);
        } SPI;
    };
    vsf_err_t                   (*Init)(i_spi_cfg_t *cfg_ptr);

    struct {
        void                    (*Set)(uintalu_t tIndex);
        void                    (*Clear)(uintalu_t tIndex);
    } CS;

    struct {
        //!< read/write of fifo
        void                    (*Transfer)(void *pOutput,
                                            uint_fast32_t* pOutCount,
                                            void *pInput,
                                            uint_fast32_t* pInCount);
        //!< flush fifo
        bool                    (*Flush)(void);
    } FIFO;

    struct {
        /*! \brief request a block exchaging access
         *! \param pOutput      address of output buffer
         *! \param pInput       address of input buffer
         *! \param nCount       the count of data in the two buffers
         *! \retval fsm_rt_cpl  The transaction is complete
         *! \retval fsm_rt_asyn The transaction is handled asynchronousely, i.e. by
         *!                     DMA or by ISR or etc.
         *! \retval fsm_rt_on_going User should poll this API until fsm_rt_cpl or err
         *!                     value is returned.
         *! \retval vsf_err_t   Error value is returned.
         */
        vsf_err_t               (*RequestTransfer)(void *pOutput, void *pInput, uint_fast32_t nCount);

        /*! \brief cancel on going communication */
        vsf_err_t               (*Cancel)(void);

        /*! \brief get transfered count */
        int_fast32_t            (*GetTransferedCount)(void);
    } Block;

    struct {
        void                    (*Enable)(em_spi_irq_mask_t mask);
        void                    (*Disable)(em_spi_irq_mask_t mask);
    } IRQ;
end_def_interface(i_spi_t)
//! @}

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern vsf_err_t        vsf_spi_init(               vsf_spi_t *spi_ptr,
                                                    spi_cfg_t *cfg_ptr);

extern fsm_rt_t         vsf_spi_enable(             vsf_spi_t *spi_ptr);
extern fsm_rt_t         vsf_spi_disable(            vsf_spi_t *spi_ptr);

extern void             vsf_spi_irq_enable(         vsf_spi_t *spi_ptr,
                                                    em_spi_irq_mask_t irq_mask);
extern void             vsf_spi_irq_disable(        vsf_spi_t *spi_ptr,
                                                    em_spi_irq_mask_t irq_mask);

extern void             vsf_spi_cs_active(          vsf_spi_t *spi_ptr,
                                                    uint_fast8_t index);
extern void             vsf_spi_cs_inactive(        vsf_spi_t *spi_ptr,
                                                    uint_fast8_t index);

extern spi_status_t     vsf_spi_status(             vsf_spi_t *spi_ptr);

extern void             vsf_spi_fifo_transfer(      vsf_spi_t *spi_ptr,
                                                    void *out_buffer_ptr,
                                                    uint_fast32_t* out_count_ptr,
                                                    void *in_buffer_ptr,
                                                    uint_fast32_t* in_count_ptr);

extern bool             vsf_spi_fifo_flush(         vsf_spi_t *spi_ptr);

extern vsf_err_t        vsf_spi_request_transfer(   vsf_spi_t *spi_ptr,
                                                    void *out_buffer_ptr,
                                                    void *in_buffer_ptr,
                                                    uint_fast32_t count);
extern vsf_err_t        vsf_spi_cancel_transfer(    vsf_spi_t *spi_ptr);
extern int_fast32_t     vsf_spi_get_transfered_count(vsf_spi_t *spi_ptr);

#endif

