/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : CLI.h
*/
#ifndef _CLI_H_
#define _CLI_H_

// If you want to change protocol just change below macros as 1 or 0
// WARNING: These macros should not be "1" at the same time
#define ENABLE_UART (0)
#define ENABLE_TCP  (1)

// Port macro
#define PORT (3333)

// TCP tranmitter and receiver buffer size macros
#define TCP_TRANSMITTED_BUFFER_SIZE (1024)
#define TCP_RECEIVED_BUFFER_SIZE  (1024)

// GPIO status macros
#define GPIO_PIN_HIGH (1)
#define GPIO_PIN_LOW  (0)

// Path macros for command history in UART
#define PROMPT_STR CONFIG_IDF_TARGET
#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

// TCP tranmitter and receiver buffers
char transmittedBuffer[TCP_TRANSMITTED_BUFFER_SIZE];
char receivedBuffer[TCP_RECEIVED_BUFFER_SIZE];

// This value (sock) is using for multiple source file so I defined it by extern keyword in other source files.
int sock; 

void cliRegisterCommands(void);
void cliConsoleInit(void);
void cliCommandControl(esp_err_t, int);
void cliInitializeNVS(void);
void cliAddCommandHistory(const char*);
void cliTCPInit(void);
void cliStartTCPScreen(void);
void cliStartUARTScreen(void);
char *cliControlConsole(void);
char *cliReadCommand(const char*);
void cliParseCommand(char*);

#endif