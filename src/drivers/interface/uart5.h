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
 * uart5.h - uart5 driver using USART5 (PC12=TX, PD2=RX)
 */
#ifndef UART5_H_
#define UART5_H_

#include <stdbool.h>
#include "eprintf.h"

#define UART5_BAUDRATE           115200
#define UART5_DATA_TIMEOUT_MS    1000
#define UART5_DATA_TIMEOUT_TICKS (UART5_DATA_TIMEOUT_MS / portTICK_RATE_MS)

#define UART5_TYPE             UART5
#define UART5_PERIF            RCC_APB1Periph_UART5
#define ENABLE_UART5_RCC       RCC_APB1PeriphClockCmd
#define UART5_IRQ              UART5_IRQn

#define UART5_DMA_IRQ          DMA1_Stream3_IRQn
#define UART5_DMA_IT_TC        DMA_IT_TC4
#define UART5_DMA_STREAM       DMA1_Stream3
#define UART5_DMA_CH           DMA_Channel_4
#define UART5_DMA_FLAG_TCIF    DMA_FLAG_TCIF3

#define UART5_GPIO_PERIF_TX    RCC_AHB1Periph_GPIOC
#define UART5_GPIO_PERIF_RX    RCC_AHB1Periph_GPIOD
#define UART5_GPIO_PORT_TX     GPIOC
#define UART5_GPIO_PORT_RX     GPIOD
#define UART5_GPIO_TX_PIN      GPIO_Pin_12
#define UART5_GPIO_RX_PIN      GPIO_Pin_2
#define UART5_GPIO_AF_TX_PIN   GPIO_PinSource12
#define UART5_GPIO_AF_RX_PIN   GPIO_PinSource2
#define UART5_GPIO_AF_TX       GPIO_AF_UART5
#define UART5_GPIO_AF_RX       GPIO_AF_UART5

#define E_CS0_GPIO_PORT    GPIOC
#define E_CS0_GPIO_PIN     GPIO_Pin_4
#define E_CS0_GPIO_PERIF   RCC_AHB1Periph_GPIOC

/**
 * Initialize the UART5 with default baudrate (UART5_BAUDRATE)
 */
void uart5Init(const uint32_t baudrate);

/**
 * Test the UART5 status.
 *
 * @return true if the UART5 is initialized
 */
bool uart5Test(void);

/**
 * Read a byte of data from incoming queue with a timeout
 * @param[out] c  Read byte
 * @param[in] timeoutTicks The timeout in sys ticks
 * @return true if data, false if timeout was reached.
 */
bool uart5GetDataWithTimeout(uint8_t *c, const uint32_t timeoutTicks);

/**
 * Read a byte of data from incoming queue with a timeout defined by UART5_DATA_TIMEOUT_MS
 * @param[out] c  Read byte
 * @return true if data, false if timeout was reached.
 */
bool uart5GetDataWithDefaultTimeout(uint8_t *c);

/**
 * Get data from the UART5 connection. Blocking until the amount of
 * data has been read
 *
 * @param[in] size  Number of bytes to read
 * @param[out] data  Pointer to data
 */
void uart5GetBytesWithDefaultTimeout(uint32_t size, uint8_t* data);

/**
 * @brief Get the number of bytes available in the UART5 in queue
 *
 * @return uint32_t  Number of bytes available
 */
uint32_t uart5bytesAvailable();

/**
 * @brief Get the maximum number of bytes that can be stored in the UART5 in queue
 *
 * @return uint32_t  The maximum length of the in queue
 */
uint32_t uart5QueueMaxLength();

/**
 * Sends raw data using a lock. Should be used from
 * exception functions and for debugging when a lot of data
 * should be transfered.
 * @param[in] size  Number of bytes to send
 * @param[in] data  Pointer to data
 */
void uart5SendData(uint32_t size, uint8_t* data);

/**
 * Sends raw data using DMA transfer.
 * @param[in] size  Number of bytes to send
 * @param[in] data  Pointer to data
 */
void uart5SendDataDmaBlocking(uint32_t size, uint8_t* data);

/**
 * Send a single character to the serial port using the uartSendData function.
 * @param[in] ch Character to print. Only the 8 LSB are used.
 *
 * @return Character printed
 */
int uart5Putchar(int ch);

void uart5Getchar(char * ch);

/**
 * Returns true if an overrun condition has happened since initialization or
 * since the last call to this function.
 *
 * @return true if an overrun condition has happened
 */
bool uart5DidOverrun();

/**
 * Uart printf macro that uses eprintf
 * @param[in] FMT String format
 * @param[in] ... Parameters to print
 */
#define uart5Printf(FMT, ...) eprintf(uart5Putchar, FMT, ## __VA_ARGS__)

#endif /* UART5_H_ */
