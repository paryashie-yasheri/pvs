#include "main.h"

#define GREEN_PIN   GPIO_PIN_13
#define RED_PIN     GPIO_PIN_14
#define YELLOW_PIN  GPIO_PIN_15

#define DEBOUNCE_MS       30U
#define LONG_PRESS_MS    600U
#define INPUT_TIMEOUT_MS 12000U
#define PULSE_MS         180U
#define OPEN_MS          3000U
#define LOCK_MS          5000U
#define BLINK_MS         250U

/* 0 — короткое нажатие, 1 — длинное.
   Код: К Д К К Д Д К Д */
static const uint8_t secret_code[8] = {0, 1, 0, 0, 1, 1, 0, 1};

typedef enum
{
  INPUT,
  OPEN_AFTER_PULSE,
  OPEN,
  LOCK_AFTER_PULSE,
  LOCK,
  TIMEOUT_PULSE
} LockState;

typedef struct
{
  GPIO_PinState raw;
  GPIO_PinState stable;
  uint32_t raw_changed_at;
  uint32_t pressed_at;
} Button;

static Button button = {GPIO_PIN_SET, GPIO_PIN_SET, 0, 0};

static LockState state = INPUT;
static uint8_t position = 0;
static uint8_t mistakes = 0;
static uint32_t first_input_at = 0;
static uint32_t state_started_at = 0;
static uint32_t blink_changed_at = 0;
static uint8_t red_is_on = 0;

static uint16_t pulse_pin = 0;
static uint32_t pulse_started_at = 0;
static uint32_t pulse_length_ms = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Драйвер светодиода: на этой плате низкий уровень включает светодиод. */
static void led_write(uint16_t pin, uint8_t on)
{
  GPIO_PinState level;

  if (pin == GREEN_PIN)
  {
    level = on ? GPIO_PIN_SET : GPIO_PIN_RESET;
  }
  else
  {
    level = on ? GPIO_PIN_RESET : GPIO_PIN_SET;
  }

  HAL_GPIO_WritePin(GPIOD, pin, level);
}
/* Драйвер кнопки: возвращает 1 один раз после отпускания кнопки. */
static uint8_t button_poll(uint32_t now, uint32_t *press_length_ms)
{
  GPIO_PinState current = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);

  if (current != button.raw)
  {
    button.raw = current;
    button.raw_changed_at = now;
  }

  if (current != button.stable &&
      (uint32_t)(now - button.raw_changed_at) >= DEBOUNCE_MS)
  {
    button.stable = current;

    if (current == GPIO_PIN_RESET)
    {
      button.pressed_at = now;
    }
    else
    {
      *press_length_ms = (uint32_t)(now - button.pressed_at);
      return 1;
    }
  }

  return 0;
}

/* Драйвер короткого светового импульса без HAL_Delay(). */
static void pulse_start(uint16_t pin, uint32_t now, uint32_t length_ms)
{
  if (pulse_pin != 0)
  {
    led_write(pulse_pin, 0);
  }

  pulse_pin = pin;
  pulse_started_at = now;
  pulse_length_ms = length_ms;
  led_write(pin, 1);
}

static void pulse_poll(uint32_t now)
{
  if (pulse_pin != 0 &&
      (uint32_t)(now - pulse_started_at) >= pulse_length_ms)
  {
    led_write(pulse_pin, 0);
    pulse_pin = 0;
  }
}

/* Принимаем одно короткое или длинное нажатие. */
static void lock_accept(uint8_t symbol, uint32_t now)
{
  if (position == 0)
  {
    first_input_at = now;
  }

  if (symbol != secret_code[position])
  {
    position = 0;
    mistakes++;
    pulse_start(RED_PIN, now, PULSE_MS);

    if (mistakes >= 3)
    {
      state = LOCK_AFTER_PULSE;
    }
    return;
  }

  position++;
  pulse_start(YELLOW_PIN, now, PULSE_MS);

  if (position == 8)
  {
    state = OPEN_AFTER_PULSE;
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  while (1)
  {
    uint32_t now = HAL_GetTick();
    uint32_t press_length_ms = 0;

    pulse_poll(now);
    uint8_t released = button_poll(now, &press_length_ms);

    switch (state)
    {
      case INPUT:
        if (position > 0 &&
            (uint32_t)(now - first_input_at) >= INPUT_TIMEOUT_MS)
        {
          position = 0;
          pulse_start(YELLOW_PIN, now, 1000U);
          state = TIMEOUT_PULSE;
        }
        else if (released)
        {
          uint8_t symbol = press_length_ms >= LONG_PRESS_MS ? 1 : 0;
          lock_accept(symbol, now);
        }
        break;

      case OPEN_AFTER_PULSE:
        if (pulse_pin == 0)
        {
          led_write(GREEN_PIN, 1);
          state_started_at = now;
          state = OPEN;
        }
        break;

      case OPEN:
        if ((uint32_t)(now - state_started_at) >= OPEN_MS)
        {
          led_write(GREEN_PIN, 0);
          position = 0;
          mistakes = 0;
          state = INPUT;
        }
        break;

      case LOCK_AFTER_PULSE:
        if (pulse_pin == 0)
        {
          state_started_at = now;
          blink_changed_at = now;
          red_is_on = 1;
          led_write(RED_PIN, red_is_on);
          state = LOCK;
        }
        break;

      case LOCK:
        if ((uint32_t)(now - state_started_at) >= LOCK_MS)
        {
          led_write(RED_PIN, 0);
          red_is_on = 0;
          position = 0;
          mistakes = 0;
          state = INPUT;
        }
        else if ((uint32_t)(now - blink_changed_at) >= BLINK_MS)
        {
          blink_changed_at = now;
          red_is_on = !red_is_on;
          led_write(RED_PIN, red_is_on);
        }
        break;

      case TIMEOUT_PULSE:
        if (pulse_pin == 0)
        {
          state = INPUT;
        }
        break;
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOD, GREEN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, RED_PIN | YELLOW_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GREEN_PIN | RED_PIN | YELLOW_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
