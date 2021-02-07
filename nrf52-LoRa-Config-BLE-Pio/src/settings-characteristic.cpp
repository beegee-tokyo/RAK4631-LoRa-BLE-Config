/**
 * @file settings-service.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handler for LoRaWAN settings service
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** LoRaWAN service 0xF0A0 */
BLEService lorawan_service = BLEService(0xF0A0);
/** LoRa settings  characteristic 0xF0A1 */
BLECharacteristic lorawan_data = BLECharacteristic(0xF0A1);

// Command callback
void settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);

/**
 * @brief Initialize the settings characteristic
 * 
 */
void init_settings_characteristic(void)
{
	// Initialize the LoRaWAN setting service
	lorawan_service.begin();
	lorawan_data.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lorawan_data.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_data.setFixedLen(sizeof(s_lorawan_settings) + 1);
	lorawan_data.setWriteCallback(settings_rx_callback);

	lorawan_data.begin();

	lorawan_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));
}

/**
 * Callback if data has been sent from the connected client
 * @param conn_hdl
 * 		The connection handle
 * @param chr
 *      The called characteristic
 * @param data
 *      Pointer to received data
 * @param len
 *      Length of the received data
 */
void settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
	myLog_d("Settings received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lorawan_data.uuid)
	{
		if (len != sizeof(s_lorawan_settings))
		{
			myLog_e("Received settings have wrong size %d", len);
			return;
		}

		s_lorawan_settings *rcvdSettings = (s_lorawan_settings *)data;
		if ((rcvdSettings->valid_mark_1 != 0xAA) || (rcvdSettings->valid_mark_2 != 0x55))
		{
			myLog_e("Received settings data do not have required markers");
			return;
		}

		// Save new LoRaWAN settings
		memcpy((void *)&g_lorawan_settings, data, sizeof(s_lorawan_settings));

		// Save new settings
		save_settings();

		// Update settings
		lorawan_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

		// Inform connected device about new settings
		lorawan_data.notify((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

		if (g_lorawan_settings.resetRequest)
		{
			myLog_d("Initiate reset");
			delay(1000);
			sd_nvic_SystemReset();
		}

		// Notify task about the event
		if (g_task_sem != NULL)
		{
			g_task_event_type = 2;
			myLog_d("Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
}