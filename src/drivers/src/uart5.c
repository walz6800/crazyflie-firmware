/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2024 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * uart5.c - uart5 driver using USART5 (PC12=TX, PD2=RX)
 */

#include "stm32fxxx.h"

/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "queue.h"

/*ST includes */
#include "stm32fxxx.h"

#include "config.h"
#include "nvic.h"
#include "uart5.h"
#include "cfassert.h"
#include "nvicconf.h"
#include "static_mem.h"

#define QUEUE_LENGTH 64
static xQueueHandle uart5queue;
STATIC_MEM_QUEUE_ALLOC(uart5queue, QUEUE_LENGTH, sizeof(uint8_t));

static bool isInit = false;
static bool hasOverrun = false;

void uart5Init(const uint32_t baudrate)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable GPIO clocks (TX on GPIOC, RX on GPIOD) and USART5 clock */
  RCC_AHB1PeriphClockCmd(UART5_GPIO_PERIF_TX, ENABLE);
  RCC_AHB1PeriphClockCmd(UART5_GPIO_PERIF_RX, ENABLE);
  ENABLE_UART5_RCC(UART5_PERIF, ENABLE);

  /* Configure USART5 Rx as input floating with pull-up */
  GPIO_InitStructure.GPIO_Pin   = UART5_GPIO_RX_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(UART5_GPIO_PORT_RX, &GPIO_InitStructure);

  /* Configure USART5 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = UART5_GPIO_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_Init(UART5_GPIO_PORT_TX, &GPIO_InitStructure);

  /* Map UART5 pins to alternate functions */
  GPIO_PinAFConfig(UART5_GPIO_PORT_TX, UART5_GPIO_AF_TX_PIN, UART5_GPIO_AF_TX);
  GPIO_PinAFConfig(UART5_GPIO_PORT_RX, UART5_GPIO_AF_RX_PIN, UART5_GPIO_AF_RX);

  /* USART5 configuration */
  USART_InitStructure.USART_BaudRate            = baudrate;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART5_TYPE, &USART_InitStructure);

  /* Configure NVIC for USART5 */
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_UART5_PRI;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Create FreeRTOS receive queue */
  uart5queue = STATIC_MEM_QUEUE_CREATE(uart5queue);

  /* Enable RXNE interrupt */
  USART_ITConfig(UART5_TYPE, USART_IT_RXNE, ENABLE);

  /* Enable USART5 */
  USART_Cmd(UART5_TYPE, ENABLE);

  isInit = true;

  /* DEBUG: Send a test string to verify UART5 hardware */
  {
    const char test[] = "\r\nUART5_INIT_OK\r\n";
    for (uint32_t i = 0; i < sizeof(test) - 1; i++) {
      while (!(UART5_TYPE->SR & USART_FLAG_TXE));
      UART5_TYPE->DR = test[i];
    }
  }
}

bool uart5Test(void)
{
  return isInit;
}

bool uart5GetDataWithTimeout(uint8_t *c, const uint32_t timeoutTicks)
{
  if (xQueueReceive(uart5queue, c, timeoutTicks) == pdTRUE)
  {
    return true;
  }

  *c = 0;
  return false;
}

bool uart5GetDataWithDefaultTimeout(uint8_t *c)
{
  return uart5GetDataWithTimeout(c, UART5_DATA_TIMEOUT_TICKS);
}

void uart5GetBytesWithDefaultTimeout(uint32_t size, uint8_t* data)
{
  for (size_t i = 0; i < size; i++) {
    xQueueReceive(uart5queue, &data[i], portMAX_DELAY);
  }
}

void uart5SendData(uint32_t size, uint8_t* data)
{
  uint32_t i;

  if (!isInit)
    return;

  for(i = 0; i < size; i++)
  {
    while (!(UART5_TYPE->SR & USART_FLAG_TXE));
    UART5_TYPE->DR = (data[i] & 0x00FF);
  }
}

int uart5Putchar(int ch)
{
    uart5SendData(1, (uint8_t *)&ch);

    return (unsigned char)ch;
}

void uart5Getchar(char * ch)
{
  xQueueReceive(uart5queue, ch, portMAX_DELAY);
}

uint32_t uart5bytesAvailable()
{
  return uxQueueMessagesWaiting(uart5queue);
}

uint32_t uart5QueueMaxLength()
{
  return QUEUE_LENGTH;
}

bool uart5DidOverrun()
{
  bool result = hasOverrun;
  hasOverrun = false;

  return result;
}

void __attribute__((used)) UART5_IRQHandler(void)
{
  if (USART_GetITStatus(UART5_TYPE, USART_IT_RXNE))
  {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t rxData = USART_ReceiveData(UART5_TYPE) & 0x00FF;
    xQueueSendFromISR(uart5queue, &rxData, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    /* If we get here, the error is most likely caused by an overrun!
     * - PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun error)
     * - and IDLE (Idle line detected) pending bits are cleared by software sequence:
     * - reading USART_SR register followed reading the USART_DR register.
     */
    asm volatile ("" : "=m" (UART5_TYPE->SR) : "r" (UART5_TYPE->SR));
    asm volatile ("" : "=m" (UART5_TYPE->DR) : "r" (UART5_TYPE->DR));

    hasOverrun = true;
  }
}
