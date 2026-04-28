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
 * uart5_pos.c - Sends lighthouse/loco positioning data over UART5
 */

#include "FreeRTOS.h"
#include "task.h"

#include "uart5_pos.h"
#include "uart5.h"
#include "stabilizer_types.h"
#include "estimator_kalman.h"
#include "static_mem.h"

#define UART5_POS_TASK_STACKSIZE 200
#define UART5_POS_TASK_PRI       1

static bool isInit = false;
static uint32_t sendPeriodMs = 50;

STATIC_MEM_TASK_ALLOC(uart5PosTask, UART5_POS_TASK_STACKSIZE);

static void uart5PosTaskFcn(void* param)
{
  point_t pos;
  uint32_t lastWakeTime = xTaskGetTickCount();
  const TickType_t periodTicks = M2T(sendPeriodMs);

  while (1) {
    vTaskDelayUntil(&lastWakeTime, periodTicks);

    // Get fused position estimate from the Kalman estimator
    // This incorporates data from lighthouse, loco, and other positioning modules
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    estimatorKalmanGetEstimatedPos(&pos);

    // Format: POS:x,y,z\r\n  (example: POS:1.234,-0.567,0.890)
    uart5Printf("POS:%.3f,%.3f,%.3f\r\n", pos.x, pos.y, pos.z);
  }
}

void uart5PositionSenderInit(const uint32_t baudrate, const uint32_t periodMs)
{
  if (isInit)
    return;

  sendPeriodMs = periodMs;

  // Initialize UART5
  uart5Init(baudrate);

  // Create the periodic position sender task
  STATIC_MEM_TASK_CREATE(uart5PosTask, uart5PosTaskFcn, "UART5POS", NULL, UART5_POS_TASK_PRI);

  isInit = true;
}

void uart5SendPosition(void)
{
  point_t pos;

  if (!isInit)
    return;

  pos.x = 0.0f;
  pos.y = 0.0f;
  pos.z = 0.0f;
  estimatorKalmanGetEstimatedPos(&pos);

  uart5Printf("POS:%.3f,%.3f,%.3f\r\n", pos.x, pos.y, pos.z);
}

bool uart5PositionSenderTest(void)
{
  return isInit;
}
