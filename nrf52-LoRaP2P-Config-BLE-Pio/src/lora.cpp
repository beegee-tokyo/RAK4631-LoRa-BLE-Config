/**
 * @file lora.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief LoRa initialization & handler
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

#ifdef _VARIANT_ISP4520_
/** DIO1 GPIO pin for ISP4520 */
#define PIN_LORA_DIO_1 11
#else
/** DIO1 GPIO pin for RAK4631 */
#define PIN_LORA_DIO_1 47
#endif

/** LoRa setting from flash */
s_lorap2p_settings g_lorap2p_settings;

/** Semaphore used by SX126x IRQ handler to wake up LoRa task */
static SemaphoreHandle_t lora_sem = NULL;

/** LoRa task handle */
TaskHandle_t loraTaskHandle;
/** GPS reading task */
void lora_task(void *pvParameters);

/** Buffer for received LoRa data */
uint8_t g_rx_lora_data[256];
/** Length of received data */
uint8_t g_rx_data_len = 0;
/** Buffer for received LoRa data */
uint8_t g_tx_lora_data[256];
/** Length of received data */
uint8_t g_tx_data_len = 0;

/** Flag if LoRa is initialized and started */
bool g_lorap2p_initialized = false;

/**************************************************************/
/* LoRa properties                                            */
/**************************************************************/
// LoRa callbacks
static RadioEvents_t RadioEvents;
void on_tx_done(void);
void on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void on_tx_timeout(void);
void on_rx_timeout(void);
void on_rx_crc_error(void);
void on_cad_done(bool cadResult);

/**
 * @brief SX126x interrupt handler
 * Called when DIO1 is set by SX126x
 * Gives lora_sem semaphore to wake up LoRa handler task
 * 
 */
void lora_interrupt_handler(void)
{
	// SX126x set IRQ
	if (lora_sem != NULL)
	{
		// Wake up LoRa task
		xSemaphoreGive(lora_sem);
	}
}

/**
 * @brief Initialize LoRa HW
 * 
 * @return int8_t result
 *  0 => OK
 * -1 => SX126x HW init failure
 * -2 => LoRa handler task start failure
 */
int8_t init_lora(void)
{
	// Create the LoRa event semaphore
	lora_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(lora_sem);

	// Initialize LoRa chip.
#ifdef _VARIANT_ISP4520_
	if (lora_isp4520_init(SX1262) != 0)
#else
	if (lora_rak4630_init() != 0)
#endif
	{
		MYLOG("LORA", "Failed to initialize SX1262");
		return -1;
	}

	// Initialize the Radio
	RadioEvents.TxDone = on_tx_done;
	RadioEvents.RxDone = on_rx_done;
	RadioEvents.TxTimeout = on_tx_timeout;
	RadioEvents.RxTimeout = on_rx_timeout;
	RadioEvents.RxError = on_rx_crc_error;
	RadioEvents.CadDone = on_cad_done;

	Radio.Init(&RadioEvents);

	Radio.Sleep(); // Radio.Standby();

	Radio.SetChannel(g_lorap2p_settings.p2p_frequency);

	Radio.SetTxConfig(MODEM_LORA, g_lorap2p_settings.p2p_tx_power, 0, g_lorap2p_settings.p2p_bandwidth,
					  g_lorap2p_settings.p2p_sf, g_lorap2p_settings.p2p_cr,
					  g_lorap2p_settings.p2p_preamble_len, false,
					  true, 0, 0, false, 5000);

	Radio.SetRxConfig(MODEM_LORA, g_lorap2p_settings.p2p_bandwidth, g_lorap2p_settings.p2p_sf,
					  g_lorap2p_settings.p2p_cr, 0, g_lorap2p_settings.p2p_preamble_len,
					  g_lorap2p_settings.p2p_symbol_timeout, false,
					  0, true, 0, 0, false, true);

	// In deep sleep we need to hijack the SX126x IRQ to trigger a wakeup of the nRF52
	attachInterrupt(PIN_LORA_DIO_1, lora_interrupt_handler, RISING);

	// Start the task that will handle the LoRa events
	MYLOG("LORA", "Starting LoRa task");
	if (!xTaskCreate(lora_task, "LORA", 4096, NULL, TASK_PRIO_LOW, &loraTaskHandle))
	{
		MYLOG("LORA", "Failed to start LoRa task");
		return -2;
	}

	// LoRa is setup, start the timer that will wakeup the loop frequently
	g_task_wakeup_timer.begin(g_lorap2p_settings.send_repeat_time, periodic_wakeup);
	g_task_wakeup_timer.start();

	Radio.Rx(0);

	digitalWrite(LED_BUILTIN, LOW);

	g_lorap2p_initialized = true;
	return 0;
}

/**
 * @brief Independent task to handle LoRa events
 * 
 * @param pvParameters Unused
 */
void lora_task(void *pvParameters)
{
	while (1)
	{
		// Switch off the indicator lights
		digitalWrite(LED_BUILTIN, LOW);
		// Only if semaphore is available we need to handle LoRa events.
		// Otherwise we sleep here until an event occurs
		if (xSemaphoreTake(lora_sem, portMAX_DELAY) == pdTRUE)
		{
			// Switch off the indicator lights
			digitalWrite(LED_BUILTIN, HIGH);
			// Handle Radio events with special process command!!!!
			Radio.IrqProcessAfterDeepSleep();
		}
	}
}

/**************************************************************/
/* LoRa callbacks                                             */
/**************************************************************/
/**
 * @brief Function to be executed on Radio Tx Done event
 */
void on_tx_done(void)
{
	MYLOG("LORA", "OnTxDone");
	// Send LoRa handler back to sleep
	xSemaphoreTake(lora_sem, 10);
	Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Done event
 */
void on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	MYLOG("LORA", "OnRxDone");

	delay(10);

	// Copy the data into loop data buffer
	memcpy(g_rx_lora_data, payload, size);
	g_rx_data_len = size;
	g_task_event_type = 0;
	// Notify task about the event
	if (g_task_sem != NULL)
	{
		MYLOG("LORA", "Waking up loop task");
		xSemaphoreGive(g_task_sem);
	}

	Radio.Rx(0);
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void on_tx_timeout(void)
{
	MYLOG("LORA", "OnTxTimeout");

	Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void on_rx_timeout(void)
{
	MYLOG("LORA", "OnRxTimeout");

	Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void on_rx_crc_error(void)
{
	Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void on_cad_done(bool cadResult)
{
	if (cadResult)
	{
		Radio.Rx(0);
	}
	else
	{
		Radio.Send(g_tx_lora_data, g_tx_data_len);
	}
}

/**
 * @brief Prepare packet to be sent and start CAD routine
 * 
 */
void send_lora_packet(void)
{
	g_tx_data_len = 0;
	g_tx_lora_data[g_tx_data_len++] = 'H';
	g_tx_lora_data[g_tx_data_len++] = 'e';
	g_tx_lora_data[g_tx_data_len++] = 'l';
	g_tx_lora_data[g_tx_data_len++] = 'l';
	g_tx_lora_data[g_tx_data_len++] = 'o';

	// Prepare LoRa CAD
	Radio.Sleep();
	Radio.SetCadParams(LORA_CAD_08_SYMBOL, g_lorap2p_settings.p2p_sf + 13, 10, LORA_CAD_ONLY, 0);

	// Switch on Indicator lights
	digitalWrite(LED_BUILTIN, HIGH);

	// Start CAD
	Radio.StartCad();
}
