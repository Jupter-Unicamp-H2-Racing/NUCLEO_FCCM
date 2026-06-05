#include "main.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart2;

void print(const char *format, ...) {
    va_list args;          // declara a lista de argumentos variáveis
    char output[200];      // buffer que vai guardar a string montada
    va_start(args, format); // inicializa a lista, começando após 'format'
    vsprintf(output, format, args); // monta a string formatada em output (ex: "Temp: 42")
    va_end(args);          // limpa a lista de argumentos
    // print_string(output); // REMOVA - duplica o envio
    HAL_UART_Transmit(&huart2, (uint8_t *)output, strlen(output), 100); // envia pelo UART
}

void println(const char *format, ...) {
    va_list args;          // declara a lista de argumentos variáveis
    char output[200];      // buffer que vai guardar a string montada
    va_start(args, format); // inicializa a lista, começando após 'format'
    vsprintf(output, format, args); // monta a string formatada em output
    va_end(args);          // limpa a lista de argumentos
    // println_string(output); // REMOVA - duplica o envio
    HAL_UART_Transmit(&huart2, (uint8_t *)output, strlen(output), 100); // envia a string
    HAL_UART_Transmit(&huart2, (uint8_t *)"\n", 1, 100); // envia a nova linha
}