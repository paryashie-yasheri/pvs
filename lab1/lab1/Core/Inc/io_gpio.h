#ifndef IO_GPIO_H
#define IO_GPIO_H

#include "stm32f4xx.h"

typedef enum
{
  IO_GPIO_INPUT = 0,
  IO_GPIO_OUTPUT = 1,
  IO_GPIO_ALTERNATE = 2,
  IO_GPIO_ANALOG = 3
} IoGpioMode;

typedef enum
{
  IO_GPIO_NO_PULL = 0,
  IO_GPIO_PULL_UP = 1,
  IO_GPIO_PULL_DOWN = 2
} IoGpioPull;

/* initial_high задает начальные уровни выходов для выбранных выводов. */
void IO_GPIO_Init(GPIO_TypeDef *port, uint16_t pins, IoGpioMode mode,
                  IoGpioPull pull, uint16_t initial_high);
IoGpioMode IO_GPIO_GetMode(GPIO_TypeDef *port, uint8_t pin_number);
uint8_t IO_GPIO_ReadPin(GPIO_TypeDef *port, uint8_t pin_number);
uint16_t IO_GPIO_Read(GPIO_TypeDef *port, uint16_t pins);
void IO_GPIO_WritePin(GPIO_TypeDef *port, uint8_t pin_number, uint8_t high);
void IO_GPIO_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t high_pins);

#endif
