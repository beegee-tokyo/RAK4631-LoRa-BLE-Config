/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief LoRaWAN configuration over BLE
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** Semaphore used by events to wake up loop task */
SemaphoreHandle_t g_task_sem = NULL;

/** Timer to wakeup task frequently and send message */
SoftwareTimer g_task_wakeup_timer;

/**
 * @brief Flag for the event type
 * -1 => no event
 * 0 => LoRaWan data received
 * 1 => Timer wakeup
 * 3 => Received configuration over BLE
 * 2 => tbd
 * ...
 */
uint8_t g_task_event_type = -1;

/**
 * @brief Timer event that wakes up the loop task frequently
 * 
 * @param unused 
 */
void periodic_wakeup(TimerHandle_t unused)
{
	// Switch on blue LED to show we are awake
	digitalWrite(LED_CONN, HIGH);
	g_task_event_type = 1;
	xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}

/**
 * @brief Arduino setup function. Called once after power-up or reset
 * 
 */
void setup()
{
	// Create the LoRaWan event semaphore
	g_task_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(g_task_sem);

	// Initialize the built in LED
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Initialize the connection status LED
	pinMode(LED_CONN, OUTPUT);
	digitalWrite(LED_CONN, HIGH);

#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
	// Initialize Serial for debug output
	Serial.begin(115200);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}

#endif

	digitalWrite(LED_BUILTIN, HIGH);

	myLog_w("=====================================");
	myLog_w("RAK4631 LoRaWan BLE Config Test");
	myLog_w("=====================================");

	// Get LoRaWAN parameter
	init_flash();

	// Init BLE
	init_ble();

	// Check if auto join is enabled
	if (g_lorawan_settings.auto_join)
	{
		myLog_d("Auto join is enabled, start LoRaWAN and join");
		// Initialize LoRaWan and start join request
		int8_t lora_init_result = init_lora();

		if (lora_init_result != 0)
		{
			myLog_d("Init LoRa failed");

			// Without working LoRa we just stop here
			while (1)
			{
				myLog_e("Nothing I can do, just loving you");
				digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
				delay(5000);
			}
		}
		myLog_d("LoRaWan init success");
	}
	else
	{
		myLog_d("Auto join is disabled, waiting for connect command");
		delay(100);
	}

	// Take the semaphore so the loop will go to sleep until an event happens
	xSemaphoreTake(g_task_sem, 10);
}

/**
 * @brief Arduino loop task. Called in a loop from the FreeRTOS task handler
 * 
 */
void loop()
{
	// Switch off blue LED to show we go to sleep
	digitalWrite(LED_BUILTIN, LOW);
	// Sleep until we are woken up by an event
	if (xSemaphoreTake(g_task_sem, portMAX_DELAY) == pdTRUE)
	{
		// Switch on blue LED to show we are awake
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500); // Only so we can see the blue LED
		switch (g_task_event_type)
		{
		case 0:
			myLog_d("Received package over LoRaWan");
			if (g_rx_lora_data[0] > 0x1F)
			{
				myLog_d("%s\n", (char *)g_rx_lora_data);
			}
			else
			{
				for (int idx = 0; idx < g_rx_data_len; idx++)
				{
					myLog_d("%X ", g_rx_lora_data[idx]);
				}
			}

			break;
		case 1:
			myLog_d("Timer wakeup");
			/// \todo read sensor or whatever you need to do frequently

			if (g_lorawan_settings.lorawan_enable)
			{ // Send the data package
				if (send_lpwan_packet())
				{
					myLog_d("LoRaWan package sent successfully");
				}
				else
				{
					myLog_d("LoRaWan package send failed");
					/// \todo maybe you need to retry here?
				}
			}
			else
			{
				send_lora_packet();
				myLog_d("LoRa package sent");
			}

			break;
		case 2:
			myLog_d("Config received over BLE");
			delay(100);

			// Inform connected device about new settings
			lorawan_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));
			lorawan_data.notify((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

			// Check if auto connect is enabled
			if ((g_lorawan_settings.auto_join) && !g_lorawan_initialized)
			{
				init_lora();
			}
			break;
		default:
			myLog_e("This should never happen ;-)");
			break;
		}
		// Go back to sleep
		xSemaphoreTake(g_task_sem, 10);
	}
}