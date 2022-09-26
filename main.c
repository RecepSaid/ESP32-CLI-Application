/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : main.c
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_system.h" 
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

// Include CLI library, it includes CLIUart and CLISocket libraries
#include "CLI.h"

// This value (sock) is using for multiple source file so I defined it by extern keyword in other source files. Extern from CLI.h
extern int sock;

// TAG for ESP32 log functions
static const char *TAGESP32 = "ESP32";
// TAG for ESP TCP log functions
static const char *TAGTCP = "TCP Application";

// CLI Task
static void cli_task(void *pvParameters){
    // If uart is enable:
    if(ENABLE_UART){
        //UART Init and Config is called in main by cliConsoleInit function
        esp_console_register_help_command();
        cliStartUARTScreen();
        // Control Console for Escape Sequences
        const char *prompt = cliControlConsole();

        // While loop for UART 
        while(1){
            // Read command from serial port line
            char *line = cliReadCommand(prompt);
            // Add the command to the history if not empty
            cliAddCommandHistory(line);
            // Parse and run the command
            cliParseCommand(line);
        }
    }
    // If tcp is enable:
    else if(ENABLE_TCP){
        // Connect to Wifi
        cliTCPInit();

        // Create structure for socket init
        char addr_str[128];
        int addr_family = (int)pvParameters;
        struct sockaddr_in dest_addr;

        // IP version and socket init
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // Create a socket
        int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0) {
            ESP_LOGE(TAGTCP, "Unable to create socket: errno %d", errno);
            goto FINISH;
        }
        ESP_LOGI(TAGTCP, "Socket created");

        // Bind socket
        int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAGTCP, "Socket unable to bind: errno %d", errno);
            goto FINISH;
        }
        ESP_LOGI(TAGTCP, "Socket bound on port %d", PORT);

        // Start socket listening
        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAGTCP, "Error occurred during listen: errno %d", errno);
            goto FINISH;
        }
        ESP_LOGI(TAGTCP, "Socket listening");

        struct sockaddr_in source_addr;
        uint addr_len = sizeof(source_addr);
        
        // Accept request which is came from client
        sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAGTCP, "Unable to accept connection: errno %d", errno);
            goto FINISH;
        }
        
        // Convert ip address to string
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        // Print client IP address
        ESP_LOGI(TAGTCP, "Socket Accepted IP Address: %s", addr_str);
        
        // Prints menu 
        cliStartTCPScreen();
        
        //While loop for TCP
        while (1) {
            // Gets command from socket
            cliReadCommand(NULL);
            // Parses command which is came from socket
            cliParseCommand(NULL);
            
            // Control for socket connection
            if (sock < 0) {
                ESP_LOGE(TAGTCP, "Unable to accept connection: errno %d", errno);
                goto FINISH;
            }
        }
        //If any error occured about socket come here
        FINISH:
        shutdown(sock, 0);
        close(sock);
        close(listen_sock);
        esp_restart();
        //vTaskDelete(NULL);  // Task can be deleted if desired
    }
    // If both are disable (uart and tcp):
    else{
        ESP_LOGE(TAGESP32, "Connection Error!\n");
        vTaskDelete(NULL);
    }
}

void app_main(void){
    // Init NVS
    cliInitializeNVS();
    // Console must be initialize for both protocol
    cliConsoleInit();
    // Register commands
    cliRegisterCommands();

    // Create CLI Task
    xTaskCreate(cli_task, "cli_task", 4096, NULL, configMAX_PRIORITIES, NULL);
}