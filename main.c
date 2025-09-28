/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : LCD I2C + Keypad 4x4 -> mini calculator
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "liquidcrystal_i2c.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ROWS 4
#define COLS 4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
/* Keypad map (calculator) */
static const char KEYS[ROWS][COLS] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','*'},
  {'C','0','=','/'}
};

/* Row/Col pins — Rows = PA0..PA3 (Output), Cols = PA4..PA7 (Input PU) */
static GPIO_TypeDef* ROW_PORTS[ROWS] = {GPIOA, GPIOA, GPIOA, GPIOA};
static uint16_t      ROW_PINS [ROWS] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
static GPIO_TypeDef* COL_PORTS[COLS] = {GPIOA, GPIOA, GPIOA, GPIOA};
static uint16_t      COL_PINS [COLS] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

/* Calculator state */
static long opA = 0, opB = 0;
static char op = 0;
static bool enteringB = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static char  keypad_get_key(void);
static void  lcd_show_expr(void);
static void  lcd_show_result(long v);
static void  lcd_show_error(const char* s);
static void  calc_reset(void);
static bool  calc_apply(long a, long b, char oper, long* out);
static void  handle_key(char k);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Scan keypad: rows=outputs (idle HIGH), cols=inputs (PU, active LOW) */
static char keypad_get_key(void)
{
  for (int r = 0; r < ROWS; r++) {
    /* set all rows HIGH, pull current row LOW */
    for (int i = 0; i < ROWS; i++)
      HAL_GPIO_WritePin(ROW_PORTS[i], ROW_PINS[i], GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_PORTS[r], ROW_PINS[r], GPIO_PIN_RESET);

    HAL_Delay(1); // settle

    for (int c = 0; c < COLS; c++) {
      if (HAL_GPIO_ReadPin(COL_PORTS[c], COL_PINS[c]) == GPIO_PIN_RESET) {
        HAL_Delay(20); // debounce
        if (HAL_GPIO_ReadPin(COL_PORTS[c], COL_PINS[c]) == GPIO_PIN_RESET) {
          /* wait release */
          while (HAL_GPIO_ReadPin(COL_PORTS[c], COL_PINS[c]) == GPIO_PIN_RESET) {
            HAL_Delay(5);
          }
          return KEYS[r][c];
        }
      }
    }
  }
  return 0;
}

static void calc_reset(void)
{
  opA = opB = 0; op = 0; enteringB = false;
  HD44780_Clear();
  HD44780_SetCursor(0,0); HD44780_PrintStr("Calc ready");
  HD44780_SetCursor(0,1); HD44780_PrintStr("Expr: ");
}

static bool calc_apply(long a, long b, char oper, long* out)
{
  switch (oper) {
    case '+': *out = a + b; return true;
    case '-': *out = a - b; return true;
    case '*': *out = a * b; return true;
    case '/': if (b==0) return false; *out = a / b; return true;
    default:  return false;
  }
}

static void lcd_show_expr(void)
{
  char line[17]; memset(line, ' ', sizeof(line)); line[16] = '\0';
  char expr[32]; expr[0] = '\0';
  char tmp[16];

  sprintf(tmp, "%ld", opA); strcat(expr, tmp);
  if (op) { size_t l = strlen(expr); expr[l] = op; expr[l+1] = '\0'; }
  if (enteringB) { sprintf(tmp, "%ld", opB); strcat(expr, tmp); }

  HD44780_SetCursor(0,1);
  snprintf(line, sizeof(line), "Expr: %-10s", expr);
  HD44780_PrintStr(line);
}

static void lcd_show_result(long v)
{
  char buf[17]; snprintf(buf, sizeof(buf), "Ans: %-10ld", v);
  HD44780_SetCursor(0,0);
  HD44780_PrintStr(buf);
}

static void lcd_show_error(const char* s)
{
  char buf[17]; snprintf(buf, sizeof(buf), "Err: %-12s", s);
  HD44780_SetCursor(0,0);
  HD44780_PrintStr(buf);
}

static void handle_key(char k)
{
  if (k == 'C') { calc_reset(); return; }

  if (k >= '0' && k <= '9') {
    if (!enteringB) {
      if (opA <= 214748364 && opA >= -214748364) opA = opA*10 + (k - '0');
    } else {
      if (opB <= 214748364 && opB >= -214748364) opB = opB*10 + (k - '0');
    }
    lcd_show_expr();
    return;
  }

  if (k=='+' || k=='-' || k=='*' || k=='/') {
    if (!op) {
      op = k; enteringB = true;
    } else if (enteringB) {
      long res;
      if (!calc_apply(opA, opB, op, &res)) { lcd_show_error("Div0"); calc_reset(); return; }
      opA = res; opB = 0; op = k;
    } else {
      op = k;
    }
    lcd_show_expr();
    return;
  }

  if (k == '=') {
    if (op && enteringB) {
      long res;
      if (!calc_apply(opA, opB, op, &res)) { lcd_show_error("Div0"); calc_reset(); return; }
      lcd_show_result(res);
      opA = res; opB = 0; op = 0; enteringB = false;
      lcd_show_expr();
    }
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  HD44780_Init(2);                 // LCD 16x2
  calc_reset();

  /* đảm bảo hàng ở mức HIGH khi idle */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    char key = keypad_get_key();
    if (key) {
      /* Hiện phím vừa bấm ở cuối dòng 2 (debug) */
      HD44780_SetCursor(10,1); char s[2]={key,0}; HD44780_PrintStr(s);
      handle_key(key);
    }
    HAL_Delay(1);
    /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

/**
  * @brief I2C1 Initialization Function
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Rows: PA0..PA3 = Output, idle HIGH */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

  /* Cols: PA4..PA7 = Input Pull-Up */
  GPIO_InitStruct.Pin  = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
