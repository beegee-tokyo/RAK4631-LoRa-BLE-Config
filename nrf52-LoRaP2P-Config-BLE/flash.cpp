/**
   @file flash.cpp
   @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
   @brief Initialize, read and write parameters from/to internal flash memory
   @version 0.1
   @date 2021-01-10

   @copyright Copyright (c) 2021

*/

#include "main.h"

s_lorap2p_settings g_flash_content;

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

static const char settings_name[] = "RAK";

File file(InternalFS);

void flash_reset(void);

/**
   @brief Initialize access to nRF52 internal file system

*/
void init_flash(void)
{
  // Initialize Internal File System
  InternalFS.begin();

  // Check if file exists
  file.open(settings_name, FILE_O_READ);
  if (!file)
  {
    MYLOG("FLASH", "File doesn't exist, force format");
    delay(100);
    flash_reset();
    return;
  }
  file.read((uint8_t *)&g_lorap2p_settings, sizeof(s_lorap2p_settings));
  file.close();
  // Check if it is LoRa P2P settings
  if ((g_lorap2p_settings.valid_mark_1 != 0xAA) || (g_lorap2p_settings.valid_mark_2 != LORA_P2P_DATA_MARKER))
  {
    // Data is not valid, reset to defaults
    MYLOG("FLASH", "Invalid data set, deleting and restart node");
    InternalFS.format();
    delay(1000);
    sd_nvic_SystemReset();
  }
  log_settings();
}

/**
   @brief Save changed settings if required

   @return boolean
  			result of saving
*/
boolean save_settings(void)
{
  bool result = true;
  // Read saved content
  file.open(settings_name, FILE_O_READ);
  if (!file)
  {
    MYLOG("FLASH", "File doesn't exist, force format");
    delay(100);
    flash_reset();
  }
  file.read((uint8_t *)&g_flash_content, sizeof(s_lorap2p_settings));
  file.close();
  if (memcmp((void *)&g_flash_content, (void *)&g_lorap2p_settings, sizeof(s_lorap2p_settings)) != 0)
  {
    MYLOG("FLASH", "Flash content changed, writing new data");
    delay(100);

    InternalFS.remove(settings_name);

    if (file.open(settings_name, FILE_O_WRITE))
    {
      file.write((uint8_t *)&g_lorap2p_settings, sizeof(s_lorap2p_settings));
      file.flush();
    }
    else
    {
      result = false;
    }
    file.close();
  }
  log_settings();
  return result;
}

/**
   @brief Reset content of the filesystem

*/
void flash_reset(void)
{
  InternalFS.format();
  if (file.open(settings_name, FILE_O_WRITE))
  {
    file.write((uint8_t *)&g_lorap2p_settings, sizeof(s_lorap2p_settings));
    file.flush();
    file.close();
  }
}

/**
   @brief Printout of all settings

*/
void log_settings(void)
{
  uint8_t index = 0;
  MYLOG("FLASH", "Saved settings:");
  MYLOG("FLASH", "%03d Marks: %02X %02X", index, g_lorap2p_settings.valid_mark_1, g_lorap2p_settings.valid_mark_2);
  index += 2;
  MYLOG("FLASH", "%03d P2P Timeout %d", index, g_lorap2p_settings.p2p_symbol_timeout);
  index += 2;
  MYLOG("FLASH", "%03d Repeat time %ld", index, g_lorap2p_settings.send_repeat_time);
  index += 4;
  MYLOG("FLASH", "%03d P2P Frequency %d", index, g_lorap2p_settings.p2p_frequency);
  index += 4;
  MYLOG("FLASH", "%03d P2P TX Power %d", index, g_lorap2p_settings.p2p_tx_power);
  index += 1;
  MYLOG("FLASH", "%03d P2P Bandwidth %d", index, g_lorap2p_settings.p2p_bandwidth);
  index += 1;
  MYLOG("FLASH", "%03d P2P SF %d", index, g_lorap2p_settings.p2p_sf);
  index += 1;
  MYLOG("FLASH", "%03d P2P CR %d", index, g_lorap2p_settings.p2p_cr);
  index += 1;
  MYLOG("FLASH", "%03d P2P Preamble %d", index, g_lorap2p_settings.p2p_preamble_len);
  index += 1;
  MYLOG("FLASH", "%03d P2P Auto Join %d", index, g_lorap2p_settings.auto_join);

  uint8_t *raw_data = (uint8_t *)&g_lorap2p_settings.valid_mark_1;
  MYLOG("FLASH", "Size %d", sizeof(s_lorap2p_settings));
  for (int idx = 0; idx < sizeof(s_lorap2p_settings); idx++)
  {
    Serial.printf("%02X ", raw_data[idx]);
  }
  Serial.println("");
  for (int idx = 0; idx < sizeof(s_lorap2p_settings); idx++)
  {
    Serial.printf("%02d ", idx);
  }
  Serial.println("");
}
