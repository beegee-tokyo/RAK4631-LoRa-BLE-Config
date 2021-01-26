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

File file(InternalFS);

void flash_reset(void);

/**
 * @brief Initialize access to nRF52 internal file system
 * 
 */
void init_flash(void)
{
  // Initialize Internal File System
  InternalFS.begin();

  // Check if file exists
  file.open("RAK", FILE_O_READ);
  if (!file)
  {
    Serial.println("File doesn't exist, force format");
    delay(100);
    flash_reset();
    return;
  }
  file.read((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
  file.close();
  log_settings();
}

/**
 * @brief Save changed settings if required
 * 
 * @return boolean 
 *       result of saving
 */
boolean save_settings(void)
{
  bool result = true;
  // Read saved content
  file.open("RAK", FILE_O_READ);
  if (!file)
  {
    Serial.println("File doesn't exist, force format");
    delay(100);
    flash_reset();
  }
  file.read((uint8_t *)&g_flash_content, sizeof(s_lorawan_settings));
  file.close();
  if (memcmp((void *)&g_flash_content, (void *)&g_lorawan_settings, sizeof(s_lorawan_settings)) != 0)
  {
    Serial.println("Flash content changed, writing new data");
    delay(100);

    InternalFS.remove("RAK");

    if (file.open("RAK", FILE_O_WRITE))
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
 * @brief Reset content of the filesystem
 * 
 */
void flash_reset(void)
{
  InternalFS.format();
  if (file.open("RAK", FILE_O_WRITE))
  {
    file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
    file.flush();
    file.close();
  }
}

/**
 * @brief Printout of all settings
 * 
 */
void log_settings(void)
{
  Serial.printf("000 Marks: %02X %02X\n", g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
  Serial.printf("002 Dev EUI %02X %02X %02X %02X %02X %02X %02X %02X\n", g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
                g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
                g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
                g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
  Serial.printf("010 App EUI %02X %02X %02X %02X %02X %02X %02X %02X\n", g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
                g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
                g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
                g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
  Serial.printf("018 App Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
			g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
			g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
			g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
			g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
			g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
			g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
			g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
  Serial.printf("036 Dev Addr %08lX", g_lorawan_settings.node_dev_addr);
  Serial.printf("040 NWS Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
			g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
			g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
			g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
			g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
			g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
			g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
			g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
  Serial.printf("056 Apps Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
			g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
			g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
			g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
			g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
			g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
			g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
			g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
  Serial.printf("072 OTAA %s\n", g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
  Serial.printf("073 ADR %s\n", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
  Serial.printf("074 %s Network\n", g_lorawan_settings.public_network ? "Public" : "Private");
  Serial.printf("075 Dutycycle %s\n", g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
  Serial.printf("076 Repeat time %ld\n", g_lorawan_settings.send_repeat_time);
  Serial.printf("080 Join trials %d\n", g_lorawan_settings.join_trials);
  Serial.printf("081 TX Power %d\n", g_lorawan_settings.tx_power);
  Serial.printf("082 DR %d\n", g_lorawan_settings.data_rate);
  Serial.printf("083 Class %d\n", g_lorawan_settings.lora_class);
  Serial.printf("084 Subband %d\n", g_lorawan_settings.subband_channels);
  Serial.printf("085 Auto join %s\n", g_lorawan_settings.auto_join ? "enabled" : "disabled");
  Serial.printf("086 Fport %d\n", g_lorawan_settings.app_port);
  Serial.printf("087 %s Message\n", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
  Serial.printf("088 Region %d\n", g_lorawan_settings.lorawan_region);
  Serial.printf("089 Mode %s\n", g_lorawan_settings.lorawan_enable ? "LoRaWAN" : "LoRa P2P");
  Serial.printf("092 P2P Frequency %d\n", g_lorawan_settings.p2p_frequency);
  Serial.printf("096 P2P TX Power %d\n", g_lorawan_settings.p2p_tx_power);
  Serial.printf("097 P2P Bandwidth %d\n", g_lorawan_settings.p2p_bandwidth);
  Serial.printf("098 P2P SF %d\n", g_lorawan_settings.p2p_sf);
  Serial.printf("099 P2P CR %d\n", g_lorawan_settings.p2p_cr);
  Serial.printf("100 P2P Preamble %d\n", g_lorawan_settings.p2p_preamble_len);
  Serial.printf("102 P2P Timeout %d\n", g_lorawan_settings.p2p_symbol_timeout);
}
