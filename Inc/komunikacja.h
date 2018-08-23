#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
#include "stdbool.h"
void collect (char c);
/**
 * plik zawierający funkcje i zmienne do obsługi transmisji przychodzącej  */

enum COMMANDS
{
ON,
GET,
ALL,
IN,
NO_CMD
};
enum READ_STATE{
       RS_START,
       RS_GET,
       RS_GET_IN,
       RS_GET_IN_VAL1,
       RS_GET_IN_ON,
       RS_GET_IN_ON_VAL2,
       RS_GET_ALL,
       RS_GET_ALL_ON,
       RS_GET_ALL_ON_VAL1,
       RS_ERROR
   };
enum COMMANDS check_cmd();
void execute_cmd();
void My_IRQ_Handler(UART_HandleTypeDef *huart);
