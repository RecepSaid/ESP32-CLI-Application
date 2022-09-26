/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : CLISocket.h
*/
#ifndef _CLISOCKET_H_
#define _CLISOCKET_H_

// For connect wifi just change SSID and Password macros
#define ESP_WIFI_SSID      "RSD"
#define ESP_WIFI_PASS      "Recep.123"
#define ESP_MAXIMUM_RETRY  100

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

void cliSocketEventHandler(void*, esp_event_base_t, int32_t, void*);
void cliSocketWifiInit(void);
void cliSocketInitTCPScreen(void);

#endif