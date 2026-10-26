#include "io_gpio.h" // Подключаем собственный интерфейс драйвера и типы CMSIS из него.

// Включает тактирование именно того GPIO-порта, указатель на который получил драйвер.
static void enable_port_clock(GPIO_TypeDef *port) // port приходит от вызывающей функции IO_GPIO_Init.
{
  if (port == GPIOA) RCC->AHB1ENR |= 1U << 0; // Для GPIOA устанавливаем бит GPIOAEN в регистре тактирования AHB1.
  else if (port == GPIOB) RCC->AHB1ENR |= 1U << 1; // Для GPIOB устанавливаем бит GPIOBEN.
  else if (port == GPIOC) RCC->AHB1ENR |= 1U << 2; // Для GPIOC устанавливаем бит GPIOCEN.
  else if (port == GPIOD) RCC->AHB1ENR |= 1U << 3; // Для GPIOD устанавливаем бит GPIODEN.
  else if (port == GPIOE) RCC->AHB1ENR |= 1U << 4; // Для GPIOE устанавливаем бит GPIOEEN.
  else if (port == GPIOF) RCC->AHB1ENR |= 1U << 5; // Для GPIOF устанавливаем бит GPIOFEN.
  else if (port == GPIOG) RCC->AHB1ENR |= 1U << 6; // Для GPIOG устанавливаем бит GPIOGEN.
  else if (port == GPIOH) RCC->AHB1ENR |= 1U << 7; // Для GPIOH устанавливаем бит GPIOHEN.
  else if (port == GPIOI) RCC->AHB1ENR |= 1U << 8; // Для GPIOI устанавливаем бит GPIOIEN.

  (void)RCC->AHB1ENR; // Читаем регистр обратно, чтобы дать периферии время принять включение тактирования.
}

// Настраивает выбранные выводы одного GPIO-порта через регистры RCC, PUPDR, MODER и BSRR.
void IO_GPIO_Init(GPIO_TypeDef *port, uint16_t pins, IoGpioMode mode, // port, mask pins и режим mode задаёт вызывающий код.
                  IoGpioPull pull, uint16_t initial_high) // pull задаёт подтяжки, initial_high — начальные единицы выходов.
{
  uint8_t pin; // Номер текущего вывода в цикле: от 0 до 15.
  uint32_t shift; // Номер младшего бита двухбитного поля вывода в MODER и PUPDR.

  enable_port_clock(port); // Включаем тактирование, прежде чем обращаться к регистрам выбранного порта.

  if (mode == IO_GPIO_OUTPUT) // Начальный уровень нужен только при настройке режима выхода.
  {
    IO_GPIO_Write(port, pins, initial_high); // Задаём latch выхода до переключения MODER, чтобы избежать неверного импульса.
  }

  for (pin = 0; pin < 16; pin++) // В GPIO-порту STM32F4 предусмотрено 16 выводов с номерами 0–15.
  {
    if ((pins & (1U << pin)) != 0) // Проверяем, установлен ли бит pin в маске от вызывающего кода.
    {
      shift = (uint32_t)pin * 2U; // У каждого вывода в этих регистрах своё поле шириной два бита.
      port->PUPDR = (port->PUPDR & ~(3U << shift)) | // Сохраняем настройки подтяжек остальных выводов порта.
                    ((uint32_t)pull << shift); // Вписываем выбранный код pull в поле текущего вывода.
      port->MODER = (port->MODER & ~(3U << shift)) | // Сохраняем режимы остальных выводов порта.
                    ((uint32_t)mode << shift); // Вписываем выбранный код mode и активируем режим вывода.
    }
  }
}

// Возвращает текущий режим одного вывода, не меняя регистры.
IoGpioMode IO_GPIO_GetMode(GPIO_TypeDef *port, uint8_t pin_number) // port и номер вывода передаёт вызывающий код.
{
  return (IoGpioMode)((port->MODER >> ((uint32_t)pin_number * 2U)) & 3U); // Сдвигаем поле вывода к младшим битам и возвращаем его код.
}

// Читает фактический логический уровень одного входа.
uint8_t IO_GPIO_ReadPin(GPIO_TypeDef *port, uint8_t pin_number) // port и номер входа передаёт вызывающий код.
{
  return (uint8_t)((port->IDR >> pin_number) & 1U); // Берём один бит из регистра входных данных IDR и возвращаем 0 или 1.
}

// Читает сразу выбранные входы, оставляя каждый уровень в бите с номером вывода.
uint16_t IO_GPIO_Read(GPIO_TypeDef *port, uint16_t pins) // Маску выводов pins передаёт вызывающий код.
{
  return (uint16_t)(port->IDR & pins); // Маской обнуляем невыбранные входы в копии IDR.
}

// Устанавливает или сбрасывает один выход атомарной записью в BSRR.
void IO_GPIO_WritePin(GPIO_TypeDef *port, uint8_t pin_number, uint8_t high) // Все три аргумента задаёт вызывающий код.
{
  uint32_t pin = 1U << pin_number; // Создаём маску с единицей только в позиции нужного вывода.
  port->BSRR = high ? pin : (pin << 16); // Нижние 16 бит BSRR включают выход, верхние 16 бит сбрасывают его.
}

// Записывает состояния нескольких выводов порта одной операцией BSRR.
void IO_GPIO_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t high_pins) // port, выбранные pins и маску единиц high_pins задаёт вызывающий код.
{
  uint32_t low_pins = (uint32_t)pins & ~(uint32_t)high_pins; // Находим выбранные выводы, которым нужно выставить ноль.
  port->BSRR = ((uint32_t)high_pins & pins) | (low_pins << 16); // В одной записи задаём выбранные единицы и нули, не затрагивая другие выводы.
}
