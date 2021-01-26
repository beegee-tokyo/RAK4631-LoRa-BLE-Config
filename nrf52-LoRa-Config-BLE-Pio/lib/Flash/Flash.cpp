
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

#if defined(NRF52) || defined(NRF52_SERIES)

#include <Arduino.h>
#include "Flash.h"

Flash::Flash()
{

}
void Flash::write(uint8_t *value, uint32_t len)
{
  uint32_t index = 0;
  uint32_t left = 0;
  uint32_t tem = 0;
  uint32_t i = 0;
  index = len/4;
  left = len%4;
  uint32_t address = FLASH_START_ADDRESS;
    // erase page.
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_NVMC->ERASEPAGE = address;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

  for(i;i<index;i++)
  {
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    tem = ((uint32_t)value[i*4] << 24) | ((uint32_t)value[1+i*4] << 16) | ((uint32_t)value[2+i*4] << 8) | ((uint32_t)value[3+i*4]);
    *(uint32_t*)address = tem;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    address+=4;
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
  }
  if(left == 0)
  {
    return;
  }
  else
  {
    address = address+4;
    if(left==3)
    {
      tem = ((uint32_t)value[i*4] << 24) | ((uint32_t)value[1+i*4] << 16) | ((uint32_t)value[2+i*4] << 8);
    }
    else if(left==2)
    {
      tem = ((uint32_t)value[i*4] << 24) | ((uint32_t)value[1+i*4] << 16);
    }
    else if(left==1)
    {
      tem = ((uint32_t)value[i*4] << 24);
    }
    else
    {
      //do nothing
    }
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    *(uint32_t*)address = tem;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    return;
  }
}

void Flash::read(uint8_t *value, uint32_t len)
{
  uint32_t index = 0;
  uint32_t left = 0;
  uint32_t tem = 0;
  uint32_t i = 0;
  index = len/4;
  left = len%4;
  uint32_t address = FLASH_START_ADDRESS;
  for(i;i<index;i++)
  {
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    tem = *(uint32_t*)address;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    value[i*4]= (tem & 0xFF000000) >> 24;
    value[1+i*4] = (tem & 0x00FF0000) >> 16; 
    value[2+i*4] = (tem & 0x0000FF00) >> 8;
    value[3+i*4] = tem & 0x000000FF;
    address+=4;        
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
  }
  if(left==0)
  {
    return;
  }
  else
  {
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    tem = *(uint32_t*)(address+4);
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    if(left==3)
    {
      value[i*4] = (tem & 0xFF000000) >> 24;
      value[1+i*4] = (tem & 0x00FF0000) >> 16; 
      value[2+i*4] = (tem & 0x0000FF00) >> 8;
    }
    else if(left==2)
    {
      value[i*4] = (tem & 0xFF000000) >> 24;
      value[1+i*4] = (tem & 0x00FF0000) >> 16; 
    }
    else if(left==1)
    {
      value[i*4] = (tem & 0xFF000000) >> 24;
    }
    else
    {
      //do nothing
    }
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    return;
  }
}


#endif // NRF52_SERIES
