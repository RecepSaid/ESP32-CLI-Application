/*
* Project: ESP32 Console Application Project - 2022
* Author : Recep Said Dulger
* File   : CLI.c
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_spi_flash.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
// Include necessary project libraries
#include "CLI.h"
#include "CLIUart.h"
#include "CLISocket.h"

// TAG for ESP32 log functions
static const char *TAGESP32 = "ESP32";
// TAG for ESP TCP log functions
static const char *TAGTCP = "TCP Application";

// All register function must be declared before
static void register_read_gpio(void);
static void register_version(void);
static void register_write_gpio(void);
static void register_restart(void);
static void register_help(void);
static void register_close_socket(void);

// Register function for all commands:
void cliRegisterCommands(void){
    register_read_gpio();
    register_write_gpio();
    register_version();
    register_restart();
#if ENABLE_TCP
    register_help();
    register_close_socket();
#endif
}

// Arguments table for 'read_gpio' command:
static struct{
    struct arg_lit *pin_param;
    struct arg_int *pin_number;
    struct arg_end *end;
}read_gpio_args;

// Command function for 'read_gpio' command:
static int read_gpio(int argc, char **argv){
    int err = arg_parse(argc, argv, (void **)&read_gpio_args);
    if(err != 0){
        //...
        return 1;
    }
    // If uart is enable:
    if(ENABLE_UART){
        // For -a argument
        if(read_gpio_args.pin_param->count){
            printf("\n -------------------- \n");
            printf("| GPIO_PIN  |  STATUS |");
            printf("\n -------------------- \n");
            for(uint8_t i = 0; i <= 39; i++){
                  //These pins not available for ESP-WROOM-32 Board
                if(i == 20 || i == 24 || i == 28 || i == 29 || i == 30 || i == 31 || i == 37 || i == 38)
                    continue;
                printf("| Pin-%d     |  %s", i, GPIO_PIN_HIGH == gpio_get_level(i) ? "HIGH   |" : "LOW    |");
                printf("\n -------------------- \n");
            }
        }
        // For -p argument
        if(read_gpio_args.pin_number->count){
            uint8_t pin = read_gpio_args.pin_number->ival[0];
            if(pin != 20 && pin != 24 && pin != 28 && pin != 29 && pin != 30 && pin != 31 && pin != 37 && pin != 38 )
                printf("GPIO Pin-%d Status: %s\n", pin, GPIO_PIN_HIGH == gpio_get_level(pin) ? "HIGH" : "LOW");
            else
                printf("This pin ( %d ) is not available in ESP-WROOM-32 Board!\n", pin); 
        }
    }
    // If tcp is enable:
    else if(ENABLE_TCP){
        // For -a argument
        if(read_gpio_args.pin_param->count){
            strcpy(transmittedBuffer, "\n------------------\n"
                                      "|GPIO_PIN | STATUS|"
                                      "\n------------------\n");
            size_t size = 0;
            for(uint8_t i = 0; i <= 39; i++){
                size = strlen(transmittedBuffer);
                //These pins not available for ESP-WRROM-32 board
                if(i == 20 || i == 24 || i == 28 || i == 29 || i == 30 || i == 31 || i == 37 || i == 38)
                    continue;
                sprintf(transmittedBuffer + size, "Pin-%d :  %s\n", i, GPIO_PIN_HIGH == gpio_get_level(i) ? "HIGH" : "LOW");
                size = strlen(transmittedBuffer);
            }
            sprintf(transmittedBuffer + size, "----------------------\n");
            int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        }
        // For -p argument
        if(read_gpio_args.pin_number->count){
            uint8_t pin = read_gpio_args.pin_number->ival[0];
            //These pins not available for ESP-WRROM-32 board
            if(pin != 20 && pin != 24 && pin != 28 && pin != 29 && pin != 30 && pin != 31 && pin != 37 && pin != 38 ){
                sprintf(transmittedBuffer, "GPIO Pin-%d Status: %s\n", pin, GPIO_PIN_HIGH == gpio_get_level(pin) ? "HIGH" : "LOW");
                int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
                if(err < 0)
                    ESP_LOGE(TAGTCP, "Error occurred during sending!");
            }
            else{
                sprintf(transmittedBuffer, "This pin ( %d ) is not available in ESP-WROOM-32 Board!\n", pin);
                int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
                if(err < 0)
                    ESP_LOGE(TAGTCP, "Error occurred during sending!");
            }
        }
    }
    // If both are disable (uart and tcp): 
    else{
        ESP_LOGE(TAGESP32, "No Connection!");
    }

    return 0;    
}

// Register function for 'read_gpio' command:
static void register_read_gpio(void){
    int num_args = 2;

    read_gpio_args.pin_param = arg_lit0("a", "allpins", "All Pins Status");
    read_gpio_args.pin_number = arg_int0("p", "pin", "<gpio>", "Pin number");
    read_gpio_args.end = arg_end(num_args);

    const esp_console_cmd_t cmd = {
        .command = "read_gpio",
        .help = "Get GPIO Pin Status",
        .hint = NULL,
        .func = &read_gpio,
        .argtable = &read_gpio_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Command function for 'version' command:
static int get_version(int argc, char **argv){
    // If uart is enable:
    if(ENABLE_UART){
        esp_chip_info_t info;
        esp_chip_info(&info);
        printf("IDF Version:%s\r\n", esp_get_idf_version());
        printf("Chip info:\r\n");
        printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknown");
        printf("\tcores:%d\r\n", info.cores);
        printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
        printf("\trevision number:%d\r\n", info.revision);
    }
    // If tcp is enable:
    else if(ENABLE_TCP){
        size_t size = 0;
        esp_chip_info_t info;
        esp_chip_info(&info);
        sprintf(transmittedBuffer + size, "IDF Version:%s\r\n", esp_get_idf_version());
        strcat(transmittedBuffer, "Chip info:\r\n");
        size = strlen(transmittedBuffer);
        sprintf(transmittedBuffer + size, "\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknown");
        size = strlen(transmittedBuffer);
        sprintf(transmittedBuffer + size, "\tcores:%d\r\n", info.cores);
        size = strlen(transmittedBuffer);
        sprintf(transmittedBuffer + size, "\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
        size = strlen(transmittedBuffer);
        sprintf(transmittedBuffer + size, "\trevision number: %d\r\n", info.revision);
        int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
        if(err < 0)
            ESP_LOGE(TAGTCP, "Error occurred during sending!");
        ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
    }
    // If both are disable (uart and tcp): 
    else{
        ESP_LOGE(TAGESP32, "No Connection!\n");
    }

    return 0;
}

// Register function for 'version' command:
static void register_version(void){
    const esp_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Arguments table for 'write_gpio' command:
static struct{
    struct arg_int *pin_number;
    struct arg_int *pin_state;
    struct arg_end *end;
}write_gpio_args;

// Command function for 'write_gpio' command:
static int write_gpio(int argc, char **argv){
    int err = arg_parse(argc, argv, (void **)&write_gpio_args);
    if(err != 0){
        //...
        return 1;
    }
    // If uart is enable:
    if(ENABLE_UART){
        int pin_number = 0;
        uint32_t pin_state = 0;
        // For -p and -d arguments
        if(write_gpio_args.pin_number->count && write_gpio_args.pin_state->count){
            pin_number = write_gpio_args.pin_number->ival[0];
            pin_state = write_gpio_args.pin_state->ival[0];
        }
        else{
            printf("-p (pin) and -d (data) argument must be entering at the same time!\n");
            return 1;
        }
        gpio_set_direction(pin_number, GPIO_MODE_INPUT_OUTPUT);
        err = gpio_set_level(pin_number, pin_state);
        if(err == ESP_OK)
            printf("Write operation successful! GPIO Pin: %d, Pin Data: %d\n", pin_number, pin_state);
        else
            printf("Fail in Writing!\n");

        }
    // If tcp is enable:    
    else if(ENABLE_TCP){
        int pin_number = 0;
        uint32_t pin_state = 0;
        // For -p and -d arguments
        if(write_gpio_args.pin_number->count && write_gpio_args.pin_state->count){
            pin_number = write_gpio_args.pin_number->ival[0];
            pin_state = write_gpio_args.pin_state->ival[0];
        }
        else{
            strcpy(transmittedBuffer, "-p (pin) and -d (data) argument must be entering at the same time!\n");
            int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));

            return 1;
        }
        gpio_set_direction(pin_number, GPIO_MODE_INPUT_OUTPUT);
        err = gpio_set_level(pin_number, pin_state);
        if(err == ESP_OK){
            sprintf(transmittedBuffer, "Write operation successful! GPIO Pin: %d, Pin Data: %d\n", pin_number, pin_state);
            int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        }
        else{
            strcpy(transmittedBuffer, "Fail during Writing!\n");
            int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        }
    }
    // If both are disable (uart and tcp):
    else{
        ESP_LOGE(TAGESP32, "No Connection!\n");
    }

    return 0;
}

// Register function for 'write_gpio' command:
static void register_write_gpio(void){
    int num_args = 2;

    write_gpio_args.pin_number = arg_int0("p", "pin", "<gpio>", "Pin number");
    write_gpio_args.pin_state = arg_int0("d", "data", "<1|0>", "Write data");
    write_gpio_args.end = arg_end(num_args);

    const esp_console_cmd_t cmd = {
        .command = "write_gpio",
        .help = "Write Data Specified GPIO Pin",
        .hint = NULL,
        .func = &write_gpio,
        .argtable = &write_gpio_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Command function for 'restart' command:
static int restart(int argc, char **argv){
    if(ENABLE_UART){
        printf("Restarting ESP32!\n");
        esp_restart();
    }
    else if(ENABLE_TCP){
        strcpy(transmittedBuffer, "Restarting ESP32!");
        int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
        if(err < 0)
            ESP_LOGE(TAGTCP, "Error occurred during sending!");
        ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        esp_restart();
    }
    else{
        ESP_LOGE(TAGESP32, "No Connection!\n");
    }

    return 0;
}

// Register function for 'restart' command:
static void register_restart(void){
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Command function for 'help' command:
static int help(int argc, char **argv){
    // This help function just for tcp:
    if(ENABLE_TCP){
        strcpy(transmittedBuffer, "\n-----------------------\n"
                                  "All Registered Commands"
                                  "\n-----------------------\n");
        strcat(transmittedBuffer, "Command: help\nHints: List All Registered Commands\n"
                                   "Arguments:\n\tNo\n\n");
        strcat(transmittedBuffer, "Command: read_gpio\nHints: Prints GPIO Status\n"
                                  "Arguments:\n\t-a : All Pins Status\n\t-p <gpio> : Specified Pin Status\n\n");
        strcat(transmittedBuffer, "Command: write_gpio\nHints: Write Desired Data in Specified Pin\n"
                                   "Arguments:\n\t-p <gpio> -d <1|0> : Pin and Data Values\n\n");
        strcat(transmittedBuffer, "Command: version\nHints: Print ESP32 Version\n"
                                   "Arguments:\n\tNo\n\n");
        strcat(transmittedBuffer, "Command: restart\nHints: Restart ESP32\n"
                                   "Arguments:\n\tNo\n\n"); 
        strcat(transmittedBuffer, "Command: close_socket\nHints: Close Socket Connection\n"
                                   "Arguments:\n\tNo\n\n");                                 
        int err = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
        if(err < 0)
            ESP_LOGE(TAGTCP, "Error occurred during sending!");
        ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
    }
    else{
        return 1;
    }

    return 0;
}

// Register function for 'help' command:
static void register_help(void){
    const esp_console_cmd_t cmd = {
        .command = "help",
        .help = "List All Registered Commands",
        .hint = NULL,
        .func = &help,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Command function for 'close_socket' command
static int close_socket(int argc, char **argv){
    // This command just for tcp
    if(ENABLE_TCP){
        // Shutdown socket
        shutdown(sock, 0);
        close(sock);
    }
    else{
        return 1;
    }

    return 0;
}

// Register function for 'close_socket' command
static void register_close_socket(void){
    const esp_console_cmd_t cmd = {
        .command = "close_socket",
        .help = "Close Socket Connection",
        .hint = NULL,
        .func = &close_socket,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// Command validity control function both TCP and UART protocol  
void cliCommandControl(esp_err_t err, int ret){
    if(ENABLE_UART){
        if(err == ESP_ERR_NOT_FOUND){
            ESP_LOGE(TAGESP32, "Unrecognized command\n");
        } 
        else if(err == ESP_ERR_INVALID_ARG){
            // command was empty
        } 
        else if(err == ESP_OK && ret != ESP_OK){
            ESP_LOGE(TAGESP32, "Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } 
        else if(err != ESP_OK){
            ESP_LOGE(TAGESP32, "Internal error: %s\n", esp_err_to_name(err));
        }
    }
    else if(ENABLE_TCP){
        if(err == ESP_ERR_NOT_FOUND){
            strcpy(transmittedBuffer, "Unrecognized command\n");
            int err2 = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err2 < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        } 
        else if(err == ESP_ERR_INVALID_ARG){
            // command was empty
        } 
        else if(err == ESP_OK && ret != ESP_OK){
            sprintf(transmittedBuffer, "Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
            int err2 = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err2 < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        } 
        else if(err != ESP_OK){
            sprintf(transmittedBuffer, "Internal error: %s\n", esp_err_to_name(err));
            int err2 = send(sock, transmittedBuffer, strlen(transmittedBuffer), 0);
            if(err2 < 0)
                ESP_LOGE(TAGTCP, "Error occurred during sending!");
            ESP_LOGI(TAGTCP, "Sending %d bytes with TCP Protocol", strlen(transmittedBuffer));
        }
    }
    else{
        ESP_LOGE(TAGESP32, "Connection Error!\n");
    }
}

// TCP init function for wifi connection
void cliTCPInit(void){
    cliSocketWifiInit();
}

// Read command function for both UART and TCP protocol
char *cliReadCommand(const char *prompt){
    if(ENABLE_UART){
        // Get a line using linenoise. The line is returned when ENTER is pressed.
        char* line = linenoise(prompt);
        if (line == NULL) { // Break on EOF or error
            return NULL;
        }

       return line;
    }
    else if(ENABLE_TCP){
        int len = recv(sock, receivedBuffer, sizeof(receivedBuffer), 0);
        if (len < 0) {
            ESP_LOGE(TAGTCP, "Error occurred during receiving: errno %d", errno);
            sock = -1;
        } 
        else if (len == 0) {
            ESP_LOGE(TAGTCP, "Connection closed");
        } 
        else {
            receivedBuffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAGESP32, "Received %d bytes: %s", len, receivedBuffer);
        }

        return NULL;
    }
    else{
        ESP_LOGE(TAGESP32, "Connection Error!\n");
        return NULL;
    }
}

// Parse command function for both UART and TCP protocol
void cliParseCommand(char *line){
    if(ENABLE_UART){
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        cliCommandControl(err, ret);
        // Linenoise allocates line buffer on the heap, so need to free it 
        linenoiseFree(line);
    }
    else if(ENABLE_TCP){
        //ESP_LOGI(TAGESP32, "receivedBuffer data before parsing: %s", receivedBuffer);
        int ret;
        esp_err_t err = esp_console_run(receivedBuffer, &ret);
        cliCommandControl(err, ret);
        if(err == ESP_OK)
            ESP_LOGI(TAGTCP, "Command Successfully Received and Processed\n");
    }
    else{
        ESP_LOGE(TAGESP32, "Connection Error!\n");
    }
}

// Start screen function for TCP protocol, it prints the menu
void cliStartTCPScreen(void){
    cliSocketInitTCPScreen();
}

// Start screen function for UART protocol, it prints the menu
void cliStartUARTScreen(void){
    cliUartInitUARTScreen();
}

// Control Console for Escape Sequences
char *cliControlConsole(void){
    static char* prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;
    
    // Figure out if the terminal supports escape sequences 
    int probe_status = linenoiseProbe();
    if (probe_status) { // Zero indicates success
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        // Since the terminal doesn't support escape sequences, don't use color codes in the prompt
        prompt = PROMPT_STR "> ";
#endif //CONFIG_LOG_COLORS
    }

return prompt;
}

// Adds commands to command history, so we can reach old command by up/down arrows
void cliAddCommandHistory(const char *line){
    if (strlen(line) > 0) {
        linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
        // Save command history to filesystem 
        linenoiseHistorySave(HISTORY_PATH);
#endif
    }
}

// Init function for NVS
void cliInitializeNVS(void){
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

// Console init function, also it calls uartConfig() function
void cliConsoleInit(void){
    // Drain stdout before reconfiguring it
    fflush(stdout);
    fsync(fileno(stdout));
    // Disable buffering on stdin
    setvbuf(stdin, NULL, _IONBF, 0);
    // Minicom, screen, idf_monitor send CR when ENTER key is pressed
    esp_vfs_dev_uart_port_set_rx_line_endings(UART_PORT, ESP_LINE_ENDINGS_CR);
    // Move the caret to the beginning of the next line on '\n'
    esp_vfs_dev_uart_port_set_tx_line_endings(UART_PORT, ESP_LINE_ENDINGS_CRLF);
    // Configure UART
    cliUartConfig();

    esp_vfs_dev_uart_use_driver(UART_PORT);
    
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
        .hint_color = atoi(LOG_COLOR_CYAN)
    };
    esp_console_init(&console_config);

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}










