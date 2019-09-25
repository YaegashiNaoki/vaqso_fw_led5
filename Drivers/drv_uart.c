/**
  ******************************************************************************
  * @file    drv_uart.c
  * @author  Merle
  * @version V1.0.0
  * @date    4-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_UART
#ifdef DBG_UART
#define UART_DMSG		RTT_Printf
#else
#define UART_DMSG(...)
#endif

#define UART_PIN_TX   BOARD_UART_TX     //28
#define UART_PIN_RX   BOARD_UART_RX     //2


/* UART instance ID. */
#define UART_INSTANCE_ID1     0
/*UART instance. */
static nrf_drv_uart_t m_uart_inst = NRF_DRV_UART_INSTANCE(UART_INSTANCE_ID1);

//static app_uart_event_handler_t   m_event_handler;            /**< Event handler function. */
static uint8_t tx_buffer[1];
static uint8_t rx_buffer[1];
static bool m_rx_ovf = false;


static uint8_t m_string_buff[NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE];
static volatile bool m_xfer_done;
static bool m_async_mode;

#define UART_RX_RING_FIFO_SIZE  512
static app_fifo_t uart_rx_ring_fifo; /**< RX FIFO buffer for storing data received on the UART until the application fetches them using app_uart_get(). */
static uint8_t uart_rx_buffer[UART_RX_RING_FIFO_SIZE];

static uint8_t b_uart_init = 0;
static uint8_t b_uart_active = 0;

static SemaphoreHandle_t xSemaphore;


static __INLINE uint32_t fifo_length(app_fifo_t * const fifo)
{
  uint32_t tmp = fifo->read_pos;
  return fifo->write_pos - tmp;
}

#define FIFO_LENGTH(F) fifo_length(&F)              /**< Macro to calculate length of a FIFO. */

static void uart_event_handler(nrf_drv_uart_event_t * p_event, void * p_context)
{
   uint32_t err_code;
    
    switch (p_event->type)
    {
        case NRF_DRV_UART_EVT_RX_DONE:
            {   
                uint8_t rx_len;
                
                // Write received byte to FIFO.
                err_code = app_fifo_put(&uart_rx_ring_fifo, p_event->data.rxtx.p_data[0]);
                if (err_code != NRF_SUCCESS)
                {
                    // Overflow in RX FIFO.
                    m_rx_ovf = true;
                    UART_DMSG("<UART> m_rx_ovf\r\n");
                }
               // else
                {
                   (void)nrf_drv_uart_rx(&m_uart_inst, rx_buffer, 1);
                }
                //err_code = nrf_drv_uart_rx(&m_uart_inst, rx_buffer, 1);
                
                b_uart_active = 1;
            }
            break;
        
        case NRF_DRV_UART_EVT_TX_DONE:
            {
                // Get next byte from FIFO.
                m_xfer_done = true;
                //RTT_Printf("<UART> TX_DONE\r\n");
                
                b_uart_active = 1;
            }
            break;
            
        case NRF_DRV_UART_EVT_ERROR:
            {
                (void)nrf_drv_uart_rx(&m_uart_inst, rx_buffer, 1);
                UART_DMSG("<UART> EVT_ERROR\r\n");
            }
            break;

        default:
            break;
    }
}


uint32_t UART_FifoFlush(void)
{
    uint32_t err_code;

    err_code = app_fifo_flush(&uart_rx_ring_fifo);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t UART_GetRxFifoLen(void)
{
    uint32_t len = UART_RX_RING_FIFO_SIZE;
    return len;
}

app_fifo_t* UART_GetRxFifoPtr(void)
{
    return &uart_rx_ring_fifo;
}



uint32_t UART_Get(uint8_t * p_byte)
{
    ASSERT(p_byte);
    bool rx_ovf = m_rx_ovf;

    ret_code_t err_code = app_fifo_get(&uart_rx_ring_fifo, p_byte);

    // If FIFO was full new request to receive one byte was not scheduled. Must be done here.
//    if (rx_ovf)
//    {
//        m_rx_ovf = false;
//        uint32_t uart_err_code = nrf_drv_uart_rx(&m_uart_inst, rx_buffer, 1);

//        // RX resume should never fail.
//        APP_ERROR_CHECK(uart_err_code);
//    }

    return err_code;
}


uint32_t m_uart_get(uint8_t * p_byte)
{
    ASSERT(p_byte);
    bool rx_ovf = m_rx_ovf;

    ret_code_t err_code;
    // If FIFO was full new request to receive one byte was not scheduled. Must be done here.
    if(rx_ovf)
    {
        m_rx_ovf = false;
        uint32_t uart_err_code = nrf_drv_uart_rx(&m_uart_inst, rx_buffer, 1);

        // RX resume should never fail.
        APP_ERROR_CHECK(uart_err_code);
    }

    return err_code;
}

uint32_t UART_PutChar(uint8_t ch)
{
    uint32_t err_code = NRF_ERROR_BUSY;

	tx_buffer[0] = ch;
	// The new byte has been added to FIFO. It will be picked up from there
	// (in 'uart_event_handler') when all preceding bytes are transmitted.
	// But if UART is not transmitting anything at the moment, we must start
	// a new transmission here.
	if (!nrf_drv_uart_tx_in_progress(&m_uart_inst))
	{
		// This operation should be almost always successful, since we've
		// just added a byte to FIFO, but if some bigger delay occurred
		// (some heavy interrupt handler routine has been executed) since
		// that time, FIFO might be empty already.
		err_code = nrf_drv_uart_tx(&m_uart_inst, tx_buffer, 1);
	}

    return err_code;
}


static void UART_AsyncTx(uint8_t const* p_buffer, uint8_t len)
{
    if (p_buffer == NULL || len == 0)
    {
        return;
    }
    uint8_t len8 = len;
    m_xfer_done = false;
    ret_code_t err_code = nrf_drv_uart_tx(&m_uart_inst, (uint8_t *)p_buffer, len8);
    APP_ERROR_CHECK(err_code);
    /* wait for completion since buffer is reused*/
    while (m_async_mode && (m_xfer_done == false))
    {
        __WFI();
    }
}

static void UART_SyncTx(uint8_t const* p_buffer, uint8_t len)
{
    if (p_buffer == NULL || len == 0)
    {
        return ;
    }
    uint8_t len8 = len;
    for (uint8_t i = 0; i < len; i ++)
    {
         while (UART_PutChar(p_buffer[i]) != NRF_SUCCESS);
    }
}

void UART_Put(uint8_t const* p_buffer, uint8_t len)
{
    if (b_uart_init == 0)
    {
        return;
    }
    if (m_async_mode)
    {
        UART_AsyncTx(p_buffer, len);
    }
    else
    {
        UART_SyncTx(p_buffer, len);
    }
}

static void UART_Fputc(uint8_t ch)
{
    if (b_uart_init == 0)
    {
        return;
    }
    if (m_async_mode)
    {
        UART_AsyncTx(&ch, 1);
    }
    else
    {
        while (UART_PutChar(ch) != NRF_SUCCESS);
    }
	
}

/*
 * after finish this func, you can use printf("");
 */
int fputc(int ch, FILE * p_file)
{
    UNUSED_PARAMETER(p_file);

    UNUSED_VARIABLE(UART_Fputc((uint8_t)ch));
    return ch;
}

void UART_Init(void)
{
	if (b_uart_init == 0)
	{
        ret_code_t err_code;
        
		static uint8_t b_semaphore_create = 0;
        if (b_semaphore_create == 0)
        {
            xSemaphore = xSemaphoreCreateMutex();
            if( xSemaphore != NULL )
            {
                /* The semaphore was created successfully and
                can be used. */
                b_semaphore_create = 1;
                
                UART_DMSG("<UART> xSemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                UART_DMSG("<UART> xSemaphoreCreateMutex  Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            } 
            
            // Configure buffer RX buffer.
            
            err_code = app_fifo_init(&uart_rx_ring_fifo, uart_rx_buffer, UART_RX_RING_FIFO_SIZE);
            APP_ERROR_CHECK(err_code);
        }


        uint8_t async_mode = true;//false;//true;
        
        nrf_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
        config.pseltxd  = UART_PIN_TX;
        config.pselrxd  = UART_PIN_RX;
        config.pselcts  = NRF_UART_PSEL_DISCONNECTED;
        config.pselrts  = NRF_UART_PSEL_DISCONNECTED;
        //config.baudrate = NRF_UART_BAUDRATE_115200;//(nrf_uart_baudrate_t)NRF_LOG_BACKEND_UART_BAUDRATE;
        //config.hwfc = NRF_UART_HWFC_DISABLED;
        //config.interrupt_priority = APP_IRQ_PRIORITY_LOW;//APP_IRQ_PRIORITY_LOWEST
        err_code = nrf_drv_uart_init(&m_uart_inst, &config, async_mode ? uart_event_handler : NULL);
        APP_ERROR_CHECK(err_code);

        m_async_mode = async_mode;
        
        if (config.pselrxd != NRF_UART_PSEL_DISCONNECTED)
        {
#ifdef UARTE_PRESENT
            if (!config.use_easy_dma)
#endif
            {
                nrf_drv_uart_rx_enable(&m_uart_inst);
            }
  
            err_code = nrf_drv_uart_rx(&m_uart_inst, rx_buffer,1);
        }
        UART_DMSG("<UART> Init.err_code =%u.\r\n", err_code);
        
        b_uart_init = 1;
    }
    else
    {
        UART_DMSG("<UART> It's already Init.\r\n");
    }
}

void UART_DeInit(void)
{
	if (b_uart_init)
	{
		b_uart_init = 0;
		nrf_drv_uart_uninit(&m_uart_inst);
        
        UART_DMSG("<UART> DeInit.\r\n");
	}
    else
    {
        UART_DMSG("<UART> It's already DeInit.\r\n");
    }
}

static nrf_drv_gpiote_pin_t uart_rx_irq_pin = UART_PIN_RX;	

static void uart_rx_gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    //static uint16_t i ;

	switch (action)
	{
		case NRF_GPIOTE_POLARITY_LOTOHI:
			
			break;
		
		case NRF_GPIOTE_POLARITY_HITOLO:
            UART_RxIrq_DeInit();
            UART_Init();
			break;
		
		case NRF_GPIOTE_POLARITY_TOGGLE:
			break;
		
		default:
			break;
	}
}


static uint8_t b_uart_rx_irq_init = 0;

void UART_RxIrq_DeInit(void)
{
    if (b_uart_rx_irq_init)
    {
        b_uart_rx_irq_init = 0;
        nrf_drv_gpiote_in_event_disable(uart_rx_irq_pin);
        nrf_drv_gpiote_in_uninit(uart_rx_irq_pin);
        UART_DMSG("<UART> RxIrq_DeInit.\r\n");
    }
    else
    {
        UART_DMSG("<UART> RxIrq_Already_DeInit.\r\n");
    }
}

void UART_RxIrq_Init(void)
{
    if (b_uart_rx_irq_init == 0)
    {
        uint32_t err_code;
        
        if (!nrf_drv_gpiote_is_init())
        {
            err_code = nrf_drv_gpiote_init();
            APP_ERROR_CHECK(err_code);
        }
        
        nrf_drv_gpiote_in_config_t key_irq_pin_cfg = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
        err_code = nrf_drv_gpiote_in_init(uart_rx_irq_pin, &key_irq_pin_cfg, uart_rx_gpiote_event_handler);
        APP_ERROR_CHECK(err_code);
        
        nrf_drv_gpiote_in_event_enable(uart_rx_irq_pin, true);
        
        UART_DMSG("<UART> RxIrq_Init.\r\n"); 
        
        b_uart_rx_irq_init = 1;
    }
    else
    {
        UART_DMSG("<UART> RxIrq_Already_Init.\r\n");
    }
}

uint8_t UART_GetInitStatus(void)
{
    uint8_t status = b_uart_init;
    return status;
}

uint8_t UART_GetActiveStatus(void)
{
    uint8_t status = b_uart_active;
    return status;
}

void UART_ClearActiveStatus(void)
{
    b_uart_active = 0;
}

#define UART_SEND_MAX_DATA_LEN      (255)

static uint8_t *packet_uart_send_buf_ptr = NULL;
static uint16_t packet_uart_send_segment_size = 0;
static uint8_t packet_uart_send_segment_remain_len = 0;
static uint16_t packet_uart_send_index = 0;
static uint8_t packet_uart_send_is_remain_state = 0;
static uint8_t packet_uart_send_is_start = 0;

void UART_PacketSend_Prepare(uint8_t const* p_send_buf, uint16_t len)
{
	packet_uart_send_buf_ptr = (uint8_t*)p_send_buf;
	packet_uart_send_segment_size = len / UART_SEND_MAX_DATA_LEN;
	packet_uart_send_segment_remain_len = len % UART_SEND_MAX_DATA_LEN;
	packet_uart_send_index = 0;
	if (packet_uart_send_segment_size == 0)
	{
		packet_uart_send_is_remain_state = 1;
	}
	else
	{
		packet_uart_send_is_remain_state = 0;
	}
	
	if (((packet_uart_send_segment_size != 0)|| (packet_uart_send_segment_remain_len != 0)) && (packet_uart_send_buf_ptr != NULL))
	{
		packet_uart_send_is_start = 1;
	}
	UART_DMSG("<UART> packet send prepare, len=%d, remain_len=%d, start=%d.\r\n",packet_uart_send_segment_size, packet_uart_send_segment_remain_len, packet_uart_send_is_start);
}


uint32_t UART_PacketSend_Loop(uint8_t *packet_send_is_finish)
{
	uint32_t err_code = NRF_SUCCESS;

	while (packet_uart_send_is_start && b_uart_init)
	{
		if (packet_uart_send_is_remain_state)
		{
			packet_uart_send_index = packet_uart_send_segment_size;
            
			UART_Put(&packet_uart_send_buf_ptr[packet_uart_send_index * UART_SEND_MAX_DATA_LEN], packet_uart_send_segment_remain_len);
            

			UART_DMSG("<UART> packet_uart_send_index =%d, Remain len = %d,Finish Send.\r\n", packet_uart_send_segment_size, packet_uart_send_segment_remain_len);
			packet_uart_send_is_start = 0;
            if (packet_send_is_finish != NULL)
            {
               (*packet_send_is_finish) = 1;
            }
		}
		else
		{
            uint16_t r_send_len = UART_SEND_MAX_DATA_LEN;
			UART_Put(&packet_uart_send_buf_ptr[packet_uart_send_index * UART_SEND_MAX_DATA_LEN], r_send_len);
            

            UART_DMSG("<UART> packet_uart_send_index =%d, Send len = %d.\r\n", packet_uart_send_index, UART_SEND_MAX_DATA_LEN);
            
            ++packet_uart_send_index;
            // 0 < 1
            if (packet_uart_send_index < packet_uart_send_segment_size)
            {

            }
            else
            {
                if (packet_uart_send_segment_remain_len)
                {
                    packet_uart_send_is_remain_state = 1;
                }
                else
                {
                    UART_DMSG("<UART> packet_uart_send_index =%d,Finish Send.\r\n", packet_uart_send_index);
                    packet_uart_send_is_start = 0;
                    if (packet_send_is_finish != NULL)
                    {
                       (*packet_send_is_finish) = 1;
                    }
                }
            }
		}
	}
    
    return err_code;
}


uint32_t UART_PacketSend(uint8_t const* p_send_buf, uint16_t len, uint8_t *p_packet_is_finish)
{
    if (b_uart_init)
    {
        xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY
        
        uint32_t err_code;
        UART_PacketSend_Prepare(p_send_buf, len);
        err_code = UART_PacketSend_Loop(p_packet_is_finish);
        
        xSemaphoreGive(xSemaphore);
    
        return err_code;
    }
    
    return NRF_ERROR_INTERNAL;
}

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
