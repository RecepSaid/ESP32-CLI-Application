/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : CLIUart.h
*/
#ifndef _CLIUART_H_
#define _CLIUART_H_

#define UART_PORT          (0)
#define UART_RX_PIN        (3)
#define UART_TX_PIN        (1)
#define UART_BAUD_RATE     (115200)
#define UART_READ_BUF_SIZE (256)

void cliUartConfig(void);
void cliUartInitUARTScreen(void);

#endif