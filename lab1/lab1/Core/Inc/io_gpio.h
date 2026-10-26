#ifndef IO_GPIO_H // Проверяем, что объявления драйвера ещё не включались в этот файл.
#define IO_GPIO_H // Устанавливаем защитный макрос, чтобы избежать повторного включения.

#include "stm32f4xx.h" // Берём из CMSIS тип GPIO_TypeDef и определения портов GPIOA–GPIOI.

// Значения режимов совпадают с кодами двухбитного поля MODER STM32F4.
typedef enum
{
  IO_GPIO_INPUT = 0,     // Вход: код 00 в поле MODER.
  IO_GPIO_OUTPUT = 1,    // Выход: код 01 в поле MODER.
  IO_GPIO_ALTERNATE = 2, // Альтернативная функция: код 10 в поле MODER.
  IO_GPIO_ANALOG = 3     // Аналоговый режим: код 11 в поле MODER.
} IoGpioMode; // Имя типа, которое используют аргументы и результат функций драйвера.

// Значения подтяжек совпадают с кодами двухбитного поля PUPDR STM32F4.
typedef enum
{
  IO_GPIO_NO_PULL = 0, // Подтяжка отключена: код 00 в поле PUPDR.
  IO_GPIO_PULL_UP = 1, // Внутренняя подтяжка вывода к питанию: код 01.
  IO_GPIO_PULL_DOWN = 2 // Внутренняя подтяжка вывода к земле: код 10.
} IoGpioPull; // Тип параметра pull в функции инициализации ниже.

// Настраивает выводы: port и маску pins передаёт вызывающий код, mode и pull выбирают конфигурацию,
// а initial_high задаёт начальные единичные уровни для выходов из маски pins.
void IO_GPIO_Init(GPIO_TypeDef *port, uint16_t pins, IoGpioMode mode, // Объявляет инициализацию порта и выбранных битами маски выводов.
                  IoGpioPull pull, uint16_t initial_high); // Продолжение списка параметров функции IO_GPIO_Init.
// Возвращает режим вывода pin_number указанного port, считанный из MODER.
IoGpioMode IO_GPIO_GetMode(GPIO_TypeDef *port, uint8_t pin_number); // port и номер вывода задаёт вызывающий код.
// Возвращает 0 или 1 для одного вывода, прочитанного из IDR.
uint8_t IO_GPIO_ReadPin(GPIO_TypeDef *port, uint8_t pin_number); // Принимает порт и номер бита внутри порта.
// Возвращает состояние выбранных выводов в исходных позициях битов маски pins.
uint16_t IO_GPIO_Read(GPIO_TypeDef *port, uint16_t pins); // Маску выбранных выводов передаёт вызывающий код.
// Устанавливает или сбрасывает один вывод по значению high (0 — низкий, ненулевое — высокий уровень).
void IO_GPIO_WritePin(GPIO_TypeDef *port, uint8_t pin_number, uint8_t high); // Принимает порт, номер вывода и логический уровень.
// Записывает уровни нескольких выводов: high_pins задаёт единицы, остальные выбранные pins — нули.
void IO_GPIO_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t high_pins); // Обе маски передаёт вызывающий код.

#endif // Конец защиты заголовка от повторного включения.
