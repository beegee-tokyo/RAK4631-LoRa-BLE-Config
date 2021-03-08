/**
   @file flash.cpp
   @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
   @brief Initialize, read and write parameters from/to internal flash memory
   @version 0.1
   @date 2021-01-10

   @copyright Copyright (c) 2021

*/

#include "main.h"

s_lorawan_settings g_flash_content;

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
  file.read((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
  file.close();
  // Check if it is LoRaWAN settings
  if ((g_lorawan_settings.valid_mark_1 != 0xAA) || (g_lorawan_settings.valid_mark_2 != LORAWAN_DATA_MARKER))
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
  file.read((uint8_t *)&g_flash_content, sizeof(s_lorawan_settings));
  file.close();
  if (memcmp((void *)&g_flash_content, (void *)&g_lorawan_settings, sizeof(s_lorawan_settings)) != 0)
  {
    MYLOG("FLASH", "Flash content changed, writing new data");
    delay(100);

    InternalFS.remove(settings_name);

    if (file.open(settings_name, FILE_O_WRITE))
    {
      file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
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
    file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
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
  MYLOG("FLASH", "%03d Marks: %02X %02X", index, g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
  index += 2;
  MYLOG("FLASH", "%03d Auto join %s", index, g_lorawan_settings.auto_join ? "enabled" : "disabled");
  index += 1;
  MYLOG("FLASH", "%03d OTAA %s", index, g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
  index += 1;
  MYLOG("FLASH", "%03d Dev EUI %02X %02X %02X %02X %02X %02X %02X %02X", index, g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
        g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
        g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
        g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
  index += 8;
  MYLOG("FLASH", "%03d App EUI %02X %02X %02X %02X %02X %02X %02X %02X", index, g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
        g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
        g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
        g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
  index += 8;
  MYLOG("FLASH", "%03d App Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        index,
        g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
        g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
        g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
        g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
        g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
        g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
        g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
        g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
  index += 16;
  MYLOG("FLASH", "%03d NWS Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        index,
        g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
        g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
        g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
        g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
        g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
        g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
        g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
        g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
  index += 16;
  MYLOG("FLASH", "%03d Apps Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        index,
        g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
        g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
        g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
        g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
        g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
        g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
        g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
        g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
  index += 16;
  MYLOG("FLASH", "%03d Dev Addr %08lX", index, g_lorawan_settings.node_dev_addr);
  index += 4;
  MYLOG("FLASH", "%03d Repeat time %ld", index, g_lorawan_settings.send_repeat_time);
  index += 4;
  MYLOG("FLASH", "%03d ADR %s", index, g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
  index += 1;
  MYLOG("FLASH", "%03d %s Network", index, g_lorawan_settings.public_network ? "Public" : "Private");
  index += 1;
  MYLOG("FLASH", "%03d Dutycycle %s", index, g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
  index += 1;
  MYLOG("FLASH", "%03d Join trials %d", index, g_lorawan_settings.join_trials);
  index += 1;
  MYLOG("FLASH", "%03d TX Power %d", index, g_lorawan_settings.tx_power);
  index += 1;
  MYLOG("FLASH", "%03d DR %d", index, g_lorawan_settings.data_rate);
  index += 1;
  MYLOG("FLASH", "%03d Class %d", index, g_lorawan_settings.lora_class);
  index += 1;
  MYLOG("FLASH", "%03d Subband %d", index, g_lorawan_settings.subband_channels);
  index += 1;
  MYLOG("FLASH", "%03d Fport %d", index, g_lorawan_settings.app_port);
  index += 1;
  MYLOG("FLASH", "%03d %s Message", index, g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");

  uint8_t *raw_data = (uint8_t *)&g_lorawan_settings.valid_mark_1;
  MYLOG("FLASH", "Size %d", sizeof(s_lorawan_settings));
  for (int idx = 0; idx < sizeof(s_lorawan_settings); idx++)
  {
    Serial.printf("%02X ", raw_data[idx]);
  }
  Serial.println("");
  for (int idx = 0; idx < sizeof(s_lorawan_settings); idx++)
  {
    Serial.printf("%02d ", idx);
  }
  Serial.println("");
}
