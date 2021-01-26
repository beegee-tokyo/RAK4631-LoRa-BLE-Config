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
extern BLECharacteristic lorawan_data;
extern BLEUart ble_uart;
extern bool ble_uart_is_connected;

// LoRa
#include <LoRaWan-RAK4630.h>
int8_t init_lora(void);
bool send_lpwan_packet(void);
void send_lora_packet(void);
extern bool lpwan_has_joined;

struct s_lorawan_settings
{
  uint8_t valid_mark_1 = 0xAA; // Just a marker for the Flash
  uint8_t valid_mark_2 = 0x55; // Just a marker for the Flash
  // OTAA Device EUI MSB
  uint8_t node_device_eui[8] = {0x00, 0x0D, 0x75, 0xE6, 0x56, 0x4D, 0xC1, 0xF3};
  // OTAA Application EUI MSB
  uint8_t node_app_eui[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x01, 0xE1};
  // OTAA Application Key MSB
  uint8_t node_app_key[16] = {0x2B, 0x84, 0xE0, 0xB0, 0x9B, 0x68, 0xE5, 0xCB, 0x42, 0x17, 0x6F, 0xE7, 0x53, 0xDC, 0xEE, 0x79};
  // ABP Device Address MSB
  uint32_t node_dev_addr = 0x26021FB4;
  // ABP Network Session Key MSB
  uint8_t node_nws_key[16] = {0x32, 0x3D, 0x15, 0x5A, 0x00, 0x0D, 0xF3, 0x35, 0x30, 0x7A, 0x16, 0xDA, 0x0C, 0x9D, 0xF5, 0x3F};
  // ABP Application Session key MSB
  uint8_t node_apps_key[16] = {0x3F, 0x6A, 0x66, 0x45, 0x9D, 0x5E, 0xDC, 0xA6, 0x3C, 0xBC, 0x46, 0x19, 0xCD, 0x61, 0xA1, 0x1E};
  bool otaa_enabled = true;                                // Flag for OTAA or ABP
  bool adr_enabled = false;                                // Flag for ADR on or off
  bool public_network = true;                              // Flag for public or private network
  bool duty_cycle_enabled = false;                         // Flag to enable duty cycle
  uint32_t send_repeat_time = 120000;                      // In milliseconds: 2 * 60 * 1000 => 2 minutes
  uint8_t join_trials = 5;                                 // Number of join retries
  uint8_t tx_power = 22;                                   // TX power 0 .. 22
  uint8_t data_rate = 3;                                   // Data rate 0 .. 15 (validity depnends on Region)
  uint8_t lora_class = 0;                                  // LoRaWAN class 0: A, 2: C, 1: B is not supported
  uint8_t subband_channels = 1;                            // Subband channel selection 1 .. 9
  bool auto_join = false;                                  // Flag if node joins automatically after reboot
  uint8_t app_port = 2;                                    // Data port to send data
  lmh_confirm confirmed_msg_enabled = LMH_UNCONFIRMED_MSG; // Flag to enable confirmed messages
  uint8_t lorawan_region = 1;                              // Fixed LoRaWAN lorawan_region (depends on compiler option
  bool lorawan_enable = true;                              // Flag for LoRaWAN or LoRa P2P
  uint32_t p2p_frequency = 923300000;                      // Frequency in Hz
  uint8_t p2p_tx_power = 22;                               // Tx power 0 .. 22
  uint8_t p2p_bandwidth = 0;                               // Bandwidth 0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved
  uint8_t p2p_sf = 7;                                      // Spreading Factor SF7..SF12
  uint8_t p2p_cr = 1;                                      // Coding Rate 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
  uint8_t p2p_preamble_len = 8;                            // Preamble length
  uint16_t p2p_symbol_timeout = 0;                         // Symbol timeout
  bool resetRequest = true;                                // Command from BLE to reset device
};

extern s_lorawan_settings g_lorawan_settings;
extern uint8_t g_rx_lora_data[];
extern uint8_t g_rx_data_len;
extern bool g_lorawan_initialized;

// Flash
void init_flash(void);
bool save_settings(void);
void log_settings(void);

#endif // MAIN_H
