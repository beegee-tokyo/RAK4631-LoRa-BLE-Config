/**
   @file lora.cpp
   @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
   @brief LoRaWAN initialization & handler
   @version 0.1
   @date 2021-01-10

   @copyright Copyright (c) 2021

*/

#include "main.h"

/** DIO1 GPIO pin for RAK4631 */
#define PIN_LORA_DIO_1 47

/** LoRaWAN setting from flash */
s_lorawan_settings g_lorawan_settings;

/** Semaphore used by SX126x IRQ handler to wake up LoRaWAN task */
static SemaphoreHandle_t lora_sem = NULL;

/** LoRa task handle */
TaskHandle_t loraTaskHandle;
/** GPS reading task */
void lora_task(void *pvParameters);

/** Buffer for received LoRaWan data */
uint8_t g_rx_lora_data[256];
/** Length of received data */
uint8_t g_rx_data_len = 0;
/** Buffer for received LoRaWan data */
uint8_t g_tx_lora_data[256];
/** Length of received data */
uint8_t g_tx_data_len = 0;

/** Flag if LoRaWAN is initialized and started */
bool g_lorawan_initialized = false;

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

/**************************************************************/
/* LoRaWAN properties                                            */
/**************************************************************/
/** LoRaWAN application data buffer. */
static uint8_t m_lora_app_data_buffer[256];
/** Lora application data structure. */
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

// LoRaWAN event handlers
/** LoRaWAN callback when join network finished */
static void lpwan_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lpwan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lpwan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lpwan_class_confirm_handler(DeviceClass_t Class);
/** LoRaWAN Function to send a package */
bool send_lpwan_packet(void);

/**@brief Structure containing LoRaWAN parameters, needed for lmh_init()

   Set structure members to
   LORAWAN_ADR_ON or LORAWAN_ADR_OFF to enable or disable adaptive data rate
   LORAWAN_DEFAULT_DATARATE OR DR_0 ... DR_5 for default data rate or specific data rate selection
   LORAWAN_PUBLIC_NETWORK or LORAWAN_PRIVATE_NETWORK to select the use of a public or private network
   JOINREQ_NBTRIALS or a specific number to set the number of trials to join the network
   LORAWAN_DEFAULT_TX_POWER or a specific number to set the TX power used
   LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF to enable or disable duty cycles
                     Please note that ETSI mandates duty cycled transmissions.
*/
static lmh_param_t lora_param_init;

/** Structure containing LoRaWan callback functions, needed for lmh_init() */
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
                                        lpwan_rx_handler, lpwan_joined_handler,
                                        lpwan_class_confirm_handler, lpwan_join_fail_handler
                                       };

bool lpwan_has_joined = false;

/**
   @brief SX126x interrupt handler
   Called when DIO1 is set by SX126x
   Gives lora_sem semaphore to wake up LoRaWan handler task

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
   @brief Initialize LoRa HW and LoRaWan MAC layer

   @return int8_t result
    0 => OK
   -1 => SX126x HW init failure
   -2 => LoRaWan MAC initialization failure
   -3 => Subband selection failure
   -4 => LoRaWan handler task start failure
*/
int8_t init_lora(void)
{
  // Create the LoRaWan event semaphore
  lora_sem = xSemaphoreCreateBinary();
  // Initialize semaphore
  xSemaphoreGive(lora_sem);

  // Initialize LoRa chip.
  if (lora_rak4630_init() != 0)
  {
    MYLOG("APP", "Failed to initialize SX1262");
    return -1;
  }

  if (g_lorawan_settings.lorawan_enable)
  {
    // Setup the EUIs and Keys
    lmh_setDevEui(g_lorawan_settings.node_device_eui);
    lmh_setAppEui(g_lorawan_settings.node_app_eui);
    lmh_setAppKey(g_lorawan_settings.node_app_key);
    lmh_setNwkSKey(g_lorawan_settings.node_nws_key);
    lmh_setAppSKey(g_lorawan_settings.node_apps_key);
    lmh_setDevAddr(g_lorawan_settings.node_dev_addr);

    // Setup the LoRaWan init structure
    lora_param_init.adr_enable = g_lorawan_settings.adr_enabled;
    lora_param_init.tx_data_rate = g_lorawan_settings.data_rate;
    lora_param_init.enable_public_network = g_lorawan_settings.public_network;
    lora_param_init.nb_trials = g_lorawan_settings.join_trials;
    lora_param_init.tx_power = g_lorawan_settings.tx_power;
    lora_param_init.duty_cycle = g_lorawan_settings.duty_cycle_enabled;

    // Initialize LoRaWan
    if (lmh_init(&lora_callbacks, lora_param_init, g_lorawan_settings.otaa_enabled) != 0)
    {
      MYLOG("APP", "Failed to initialize LoRaWAN");
      return -2;
    }

    // For some regions we might need to define the sub band the gateway is listening to
    // This must be called AFTER lmh_init()
    if (!lmh_setSubBandChannels(g_lorawan_settings.subband_channels))
    {
      MYLOG("APP", "lmh_setSubBandChannels failed. Wrong sub band requested?");
      return -3;
    }

    // Start the task that will handle the LoRaWan events
    MYLOG("APP", "Starting LoRaWan task");
    if (!xTaskCreate(lora_task, "LORA", 4096, NULL, TASK_PRIO_LOW, &loraTaskHandle))
    {
      MYLOG("APP", "Failed to start LoRaWAN task");
      return -4;
    }

    // Start Join procedure
    MYLOG("APP", "Start network join request");
    lmh_join();
  }
  else
  {
    // Initialize the Radio
    RadioEvents.TxDone = on_tx_done;
    RadioEvents.RxDone = on_rx_done;
    RadioEvents.TxTimeout = on_tx_timeout;
    RadioEvents.RxTimeout = on_rx_timeout;
    RadioEvents.RxError = on_rx_crc_error;
    RadioEvents.CadDone = on_cad_done;

    Radio.Init(&RadioEvents);

    Radio.Sleep(); // Radio.Standby();

    Radio.SetChannel(g_lorawan_settings.p2p_frequency);

    Radio.SetTxConfig(MODEM_LORA, g_lorawan_settings.p2p_tx_power, 0, g_lorawan_settings.p2p_bandwidth,
                      g_lorawan_settings.p2p_sf, g_lorawan_settings.p2p_cr,
                      g_lorawan_settings.p2p_preamble_len, false,
                      true, 0, 0, false, 5000);

    Radio.SetRxConfig(MODEM_LORA, g_lorawan_settings.p2p_bandwidth, g_lorawan_settings.p2p_sf,
                      g_lorawan_settings.p2p_cr, 0, g_lorawan_settings.p2p_preamble_len,
                      g_lorawan_settings.p2p_symbol_timeout, false,
                      0, true, 0, 0, false, true);


    // Start the task that will handle the LoRaWan events
    MYLOG("APP", "Starting LoRa task");
    if (!xTaskCreate(lora_task, "LORA", 4096, NULL, TASK_PRIO_LOW, &loraTaskHandle))
    {
      MYLOG("APP", "Failed to start LoRa task");
      return -4;
    }

    // LoRa is setup, start the timer that will wakeup the loop frequently
    g_task_wakeup_timer.begin(g_lorawan_settings.send_repeat_time, periodic_wakeup);
    g_task_wakeup_timer.start();

    Radio.Rx(0);

    digitalWrite(LED_BUILTIN, LOW);

    return 0;
  }

  g_lorawan_initialized = true;
  return 0;
}

/**
   @brief Independent task to handle LoRa events

   @param pvParameters Unused
*/
void lora_task(void *pvParameters)
{
  while (1)
  {
    if ((g_lorawan_settings.lorawan_enable) && !lpwan_has_joined)
    {
      Radio.IrqProcess();
      delay(100);
      if (lpwan_has_joined)
      {
        // In deep sleep we need to hijack the SX126x IRQ to trigger a wakeup of the nRF52
        attachInterrupt(PIN_LORA_DIO_1, lora_interrupt_handler, RISING);
      }
    }
    else
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
}

/**************************************************************/
/* LoRaWAN callback functions                                 */
/**************************************************************/
/**
   @brief LoRa function when join has failed
*/
void lpwan_join_fail_handler (void)
{
  MYLOG("APP", "OTAA joined failed");
  MYLOG("APP", "Check LPWAN credentials and if a gateway is in range");
}

/**
   @brief LoRa function for handling HasJoined event.
*/
static void lpwan_joined_handler(void)
{
  digitalWrite(LED_BUILTIN, LOW);

  if (g_lorawan_settings.otaa_enabled)
  {
    uint32_t otaaDevAddr = lmh_getDevAddr();
    MYLOG("APP", "OTAA joined and got dev address %08X", otaaDevAddr);
  }
  else
  {
    MYLOG("APP", "ABP joined");
  }

  // Class A is default in the LoRaWAN lib. If app needs different class, request change here
  if (g_lorawan_settings.lora_class != CLASS_A)
  {
    // Switch to configured class
    lmh_class_request((DeviceClass_t)g_lorawan_settings.lora_class);
  }
  else
  {
    // Wake up task to send initial packet
    g_task_event_type = 1;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
      MYLOG("APP", "Waking up loop task");
      xSemaphoreGive(g_task_sem);
    }

    lpwan_has_joined = true;
  }

  // Now we are connected, start the timer that will wakeup the loop frequently
  g_task_wakeup_timer.begin(g_lorawan_settings.send_repeat_time, periodic_wakeup);
  g_task_wakeup_timer.start();
}

/**
   @brief Function for handling LoRaWan received data from Gateway

   @param app_data  Pointer to rx data
*/
static void lpwan_rx_handler(lmh_app_data_t *app_data)
{
  MYLOG("APP", "LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d",
        app_data->port, app_data->buffsize, app_data->rssi, app_data->snr);

  switch (app_data->port)
  {
    case 3:
      // Port 3 switches the class
      if (app_data->buffsize == 1)
      {
        switch (app_data->buffer[0])
        {
          case 0:
            lmh_class_request(CLASS_A);
            MYLOG("APP", "Request to switch to class A");
            break;

          case 1:
            lmh_class_request(CLASS_B);
            MYLOG("APP", "Request to switch to class B");
            break;

          case 2:
            lmh_class_request(CLASS_C);
            MYLOG("APP", "Request to switch to class C");
            break;

          default:
            break;
        }
      }
      break;
    case LORAWAN_APP_PORT:
      // Copy the data into loop data buffer
      memcpy(g_rx_lora_data, app_data->buffer, app_data->buffsize);
      g_rx_data_len = app_data->buffsize;
      g_task_event_type = 0;
      // Notify task about the event
      if (g_task_sem != NULL)
      {
        MYLOG("APP", "Waking up loop task");
        xSemaphoreGive(g_task_sem);
      }
  }
}

/**
   @brief Callback for class switch confirmation

   @param Class The new class
*/
static void lpwan_class_confirm_handler(DeviceClass_t Class)
{
  MYLOG("APP", "switch to class %c done", "ABC"[Class]);

  // Wake up task to send initial packet
  g_task_event_type = 1;
  // Notify task about the event
  if (g_task_sem != NULL)
  {
    MYLOG("APP", "Waking up loop task");
    xSemaphoreGive(g_task_sem);
  }
  lpwan_has_joined = true;
}

/**
   @brief Send a LoRaWan package

   @return result of send request
*/
bool send_lpwan_packet(void)
{
  if (g_lorawan_settings.lorawan_enable)
  {
    if (lmh_join_status_get() != LMH_SET)
    {
      //Not joined, try again later
      MYLOG("APP", "Did not join network, skip sending frame");
      return false;
    }

    m_lora_app_data.port = LORAWAN_APP_PORT;

    /// \todo here some more usefull data should be put into the package
    uint8_t buffSize = 0;
    m_lora_app_data_buffer[buffSize++] = 'H';
    m_lora_app_data_buffer[buffSize++] = 'e';
    m_lora_app_data_buffer[buffSize++] = 'l';
    m_lora_app_data_buffer[buffSize++] = 'l';
    m_lora_app_data_buffer[buffSize++] = 'o';

    m_lora_app_data.buffsize = buffSize;

    lmh_error_status error = lmh_send(&m_lora_app_data, g_lorawan_settings.confirmed_msg_enabled);

    return (error == 0);
  }
  else
  {
    return true;
  }
}

/**************************************************************/
/* LoRa properties                                            */
/**************************************************************/
/**
   @brief Function to be executed on Radio Tx Done event
*/
void on_tx_done(void)
{
  MYLOG("APP", "OnTxDone");
  // Send LoRa handler back to sleep
  xSemaphoreTake(lora_sem, 10);
  Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Done event
*/
void on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  MYLOG("APP", "OnRxDone");

  delay(10);

  // Copy the data into loop data buffer
  memcpy(g_rx_lora_data, payload, size);
  g_rx_data_len = size;
  g_task_event_type = 0;
  // Notify task about the event
  if (g_task_sem != NULL)
  {
    MYLOG("APP", "Waking up loop task");
    xSemaphoreGive(g_task_sem);
  }

  Radio.Rx(0);
}

/**@brief Function to be executed on Radio Tx Timeout event
*/
void on_tx_timeout(void)
{
  MYLOG("APP", "OnTxTimeout");

  Radio.Rx(0);
}

/**@brief Function to be executed on Radio Rx Timeout event
*/
void on_rx_timeout(void)
{
  MYLOG("APP", "OnRxTimeout");

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
   @brief Prepare packet to be sent and start CAD routine

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
  Radio.SetCadParams(LORA_CAD_08_SYMBOL, g_lorawan_settings.p2p_sf + 13, 10, LORA_CAD_ONLY, 0);

  // Switch on Indicator lights
  digitalWrite(LED_CONN, HIGH);

  // Start CAD
  Radio.StartCad();
}
