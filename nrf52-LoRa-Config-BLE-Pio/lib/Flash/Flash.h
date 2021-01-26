/*
 * Flash library for nRF5x
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
 * Copyright (c) 2016 Sandeep Mistry All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef Flash_h
#define Flash_h

#include <Arduino.h>
//We exploit 4096 Bytes for user to store their data
//Start address is 0xE0000, range is 0xE0000-0xE1000, near to Bootloader
#define FLASH_START_ADDRESS     0xE0000

class Flash
{
  public:
    Flash();
    void write(uint8_t *value, uint32_t len);
    void read(uint8_t *value, uint32_t len);  

};

#endif
