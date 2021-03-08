/**
   @file main.h
   @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
   @brief Includes and global declarations
   @version 0.1
   @date 2021-01-10

   @copyright Copyright (c) 2021

*/
#ifndef MAIN_H
#define MAIN_H

#define SW_VERSION 0.01

// Debug output set to 0 to disable app debug output
#define MY_DEBUG 1

#if MY_DEBUG > 0
#define MYLOG(tag, ...)           \
  do                            \
  {                             \
    if (tag)                  \
      PRINTF("[%s] ", tag); \
    PRINTF(__VA_ARGS__);      \
    PRINTF("\n");             \
  } while (0)
#else
#define MYLOG(...)
#endif

#include <Arduino.h>
#include <nrf_nvic.h>

// Main loop stuff
void periodic_wakeup(TimerHandle_t unused);
extern SemaphoreHandle_t g_task_sem;
extern uint8_t g_task_event_type;
extern SoftwareTimer g_task_wakeup_timer;

// BLE
#include <bluefruit.h>
void init_ble(void);
void init_settings_characteristic(void);
extern BLECharacteristic lora_data;
extern BLEUart ble_uart;
extern bool ble_uart_is_connected;

// LoRa
#include <SX126x-RAK4630.h>
int8_t init_lora(void);
bool send_lpwan_packet(void);
void send_lora_packet(void);
extern bool lpwan_has_joined;

#define LORA_P2P_DATA_MARKER 0x56
struct s_lorap2p_settings
{
  uint8_t valid_mark_1 = 0xAA; // Just a marker for the Flash
  uint8_t valid_mark_2 = LORA_P2P_DATA_MARKER; // Just a marker for the Flash

  // OTAA Device EUI MSB
  // Symbol timeout
  uint16_t p2p_symbol_timeout = 0;
  // Send repeat time in milliseconds: 2 * 60 * 1000 => 2 minutes
  uint32_t send_repeat_time = 120000;
  // Frequency in Hz
  uint32_t p2p_frequency = 923300000;
  // Tx power 0 .. 22
  uint8_t p2p_tx_power = 22;
  // Bandwidth 0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved
  uint8_t p2p_bandwidth = 0;
  // Spreading Factor SF7..SF12
  uint8_t p2p_sf = 7;
  // Coding Rate 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
  uint8_t p2p_cr = 1;
  // Preamble length
  uint8_t p2p_preamble_len = 8;
  // Flag if node joins automatically after reboot
  bool auto_join = false;
  // Command from BLE to reset device
  bool resetRequest = true;
};

extern s_lorap2p_settings g_lorap2p_settings;
extern uint8_t g_rx_lora_data[];
extern uint8_t g_rx_data_len;
extern bool g_lorap2p_initialized;

// Flash
void init_flash(void);
bool save_settings(void);
void log_settings(void);

#endif // MAIN_H
