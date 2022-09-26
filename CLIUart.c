/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : CLIUart.c
*/
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "CLIUart.h"

// Uart config function
void cliUartConfig(void){
    // Control value
    int err = 0;
    // Uart configuration 
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE
    };
    // Uart parameters configuration function
    err = uart_param_config(UART_PORT, &uart_config);
    if(err == ESP_OK)
        printf(">UART Configuration Successful!\n");
    else
        printf(">UART Configuration Fail!\n");
    // Uart set pins function
    err = uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if(err == ESP_OK)
        printf(">UART Set Pin Successful!\n");
    else
        printf(">UART Set Pin Fail!\n");
    // Uart driver intallation function
    err = uart_driver_install(UART_PORT, UART_READ_BUF_SIZE, 0, 0, NULL, 0);
    if(err == ESP_OK)
        printf(">UART Driver Install Successful!\n");
    else
        printf(">UART Driver Install Fail!\n");    
}

// Start screen function for UART protocol, it prints the menu
void cliUartInitUARTScreen(void){
    printf("\n=============== ESP32 Console Application ===============\n"
           "Type 'help' to Get the List of Commands\n"
           "Use UP/DOWN Arrows to Navigate Through Command History\n"
           "Press TAB When Typing Command Name to Auto-Complete\n"
           "Press Enter or Ctrl+C Will Terminate the Console Environment\n"
           "===========================================================\n");
}

