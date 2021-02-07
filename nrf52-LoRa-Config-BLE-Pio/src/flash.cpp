/**
 * @file flash.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize, read and write parameters from/to internal flash memory
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
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
 * @brief Initialize access to nRF52 internal file system
 * 
 */
void init_flash(void)
{
	// Initialize Internal File System
	InternalFS.begin();

	// Check if file exists
	file.open(settings_name, FILE_O_READ);
	if (!file)
	{
		myLog_e("File doesn't exist, force format");
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
 * 			result of saving
 */
boolean save_settings(void)
{
	bool result = true;
	// Read saved content
	file.open(settings_name, FILE_O_READ);
	if (!file)
	{
		myLog_e("File doesn't exist, force format");
		delay(100);
		flash_reset();
	}
	file.read((uint8_t *)&g_flash_content, sizeof(s_lorawan_settings));
	file.close();
	if (memcmp((void *)&g_flash_content, (void *)&g_lorawan_settings, sizeof(s_lorawan_settings)) != 0)
	{
		myLog_e("Flash content changed, writing new data");
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
 * @brief Reset content of the filesystem
 * 
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
 * @brief Printout of all settings
 * 
 */
void log_settings(void)
{
	myLog_d("000 Marks: %02X %02X", g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
	myLog_d("002 Dev EUI %02X %02X %02X %02X %02X %02X %02X %02X ",
			g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
			g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
			g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
			g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
	myLog_d("010 App EUI %02X %02X %02X %02X %02X %02X %02X %02X ",
			g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
			g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
			g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
			g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
	myLog_d("018 App Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
			g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
			g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
			g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
			g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
			g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
			g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
			g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
			g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
	myLog_d("036 Dev Addr %08lX", g_lorawan_settings.node_dev_addr);
	myLog_d("040 NWS Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
			g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
			g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
			g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
			g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
			g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
			g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
			g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
			g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
	myLog_d("056 Apps Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
			g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
			g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
			g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
			g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
			g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
			g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
			g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
			g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
	myLog_d("072 OTAA %s", g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
	myLog_d("073 ADR %s", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
	myLog_d("074 %s Network", g_lorawan_settings.public_network ? "Public" : "Private");
	myLog_d("075 Dutycycle %s", g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
	myLog_d("076 Repeat time %ld", g_lorawan_settings.send_repeat_time);
	myLog_d("080 Join trials %d", g_lorawan_settings.join_trials);
	myLog_d("081 TX Power %d", g_lorawan_settings.tx_power);
	myLog_d("082 DR %d", g_lorawan_settings.data_rate);
	myLog_d("083 Class %d", g_lorawan_settings.lora_class);
	myLog_d("084 Subband %d", g_lorawan_settings.subband_channels);
	myLog_d("085 Auto join %s", g_lorawan_settings.auto_join ? "enabled" : "disabled");
	myLog_d("086 Fport %d", g_lorawan_settings.app_port);
	myLog_d("087 %s Message", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
	myLog_d("088 Region %d", g_lorawan_settings.lorawan_region);
	myLog_d("089 Mode %s", g_lorawan_settings.lorawan_enable ? "LoRaWAN" : "LoRa P2P");
	myLog_d("092 P2P Frequency %d", g_lorawan_settings.p2p_frequency);
	myLog_d("096 P2P TX Power %d", g_lorawan_settings.p2p_tx_power);
	myLog_d("097 P2P Bandwidth %d", g_lorawan_settings.p2p_bandwidth);
	myLog_d("098 P2P SF %d", g_lorawan_settings.p2p_sf);
	myLog_d("099 P2P CR %d", g_lorawan_settings.p2p_cr);
	myLog_d("100 P2P Preamble %d", g_lorawan_settings.p2p_preamble_len);
	myLog_d("102 P2P Timeout %d", g_lorawan_settings.p2p_symbol_timeout);
}