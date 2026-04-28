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
 * uart5_pos.h - Position data sender over UART5
 */
#ifndef UART5_POS_H_
#define UART5_POS_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the UART5 position sender.
 * Creates a FreeRTOS task that periodically sends position data over UART5.
 * @param baudrate  UART5 baudrate (e.g. 115200)
 * @param periodMs  Send period in milliseconds (e.g. 50 for 20Hz)
 */
void uart5PositionSenderInit(const uint32_t baudrate, const uint32_t periodMs);

/**
 * Test whether the position sender is initialized.
 * @return true if initialized
 */
bool uart5PositionSenderTest(void);

/**
 * Send current position estimate over UART5 immediately.
 * Can be called outside of the periodic task.
 * Format: "POS:x,y,z\r\n" where x, y, z are in meters with 3 decimal places.
 */
void uart5SendPosition(void);

#endif /* UART5_POS_H_ */
