#include "io_gpio.h"

static void enable_port_clock(GPIO_TypeDef *port)
{
  if (port == GPIOA) RCC->AHB1ENR |= 1U << 0;
  else if (port == GPIOB) RCC->AHB1ENR |= 1U << 1;
  else if (port == GPIOC) RCC->AHB1ENR |= 1U << 2;
  else if (port == GPIOD) RCC->AHB1ENR |= 1U << 3;
  else if (port == GPIOE) RCC->AHB1ENR |= 1U << 4;
  else if (port == GPIOF) RCC->AHB1ENR |= 1U << 5;
  else if (port == GPIOG) RCC->AHB1ENR |= 1U << 6;
  else if (port == GPIOH) RCC->AHB1ENR |= 1U << 7;
  else if (port == GPIOI) RCC->AHB1ENR |= 1U << 8;

  (void)RCC->AHB1ENR;
}

void IO_GPIO_Init(GPIO_TypeDef *port, uint16_t pins, IoGpioMode mode,
                  IoGpioPull pull, uint16_t initial_high)
{
  uint8_t pin;
  uint32_t shift;

  enable_port_clock(port);

  if (mode == IO_GPIO_OUTPUT)
  {
    IO_GPIO_Write(port, pins, initial_high);
  }

  for (pin = 0; pin < 16; pin++)
  {
    if ((pins & (1U << pin)) != 0)
    {
      shift = (uint32_t)pin * 2U;
      port->PUPDR = (port->PUPDR & ~(3U << shift)) |
                    ((uint32_t)pull << shift);
      port->MODER = (port->MODER & ~(3U << shift)) |
                    ((uint32_t)mode << shift);
    }
  }
}

IoGpioMode IO_GPIO_GetMode(GPIO_TypeDef *port, uint8_t pin_number)
{
  return (IoGpioMode)((port->MODER >> ((uint32_t)pin_number * 2U)) & 3U);
}

uint8_t IO_GPIO_ReadPin(GPIO_TypeDef *port, uint8_t pin_number)
{
  return (uint8_t)((port->IDR >> pin_number) & 1U);
}

uint16_t IO_GPIO_Read(GPIO_TypeDef *port, uint16_t pins)
{
  return (uint16_t)(port->IDR & pins);
}

void IO_GPIO_WritePin(GPIO_TypeDef *port, uint8_t pin_number, uint8_t high)
{
  uint32_t pin = 1U << pin_number;
  port->BSRR = high ? pin : (pin << 16);
}

void IO_GPIO_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t high_pins)
{
  uint32_t low_pins = (uint32_t)pins & ~(uint32_t)high_pins;
  port->BSRR = ((uint32_t)high_pins & pins) | (low_pins << 16);
}
