/* USER CODE BEGIN Header */
	/**
	  ******************************************************************************
	  * @file           : main.c
	  * @brief          : ARC 2026 - Direct Button Logic (No Timers) | LED PB8
	  ******************************************************************************
	  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
	#include <math.h>
	#include "generated_layouts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
	// I2C Address
	#define TCS34725_ADDR        (0x29 << 1)

	// Command bit
	#define TCS34725_COMMAND_BIT 0x80
	#define TCS34725_AUTO_INC    0x20

	// Registers
	#define TCS34725_ENABLE      0x00
	#define TCS34725_ATIME       0x01
	#define TCS34725_CONTROL     0x0F

	// RGBC data registers
	#define TCS34725_CDATAL      0x14
	#define TCS34725_RDATAL      0x16
	#define TCS34725_GDATAL      0x18
	#define TCS34725_BDATAL      0x1A

	#define SAMPLES  15
	#define MAX_COLORS 8

	#define COLOR_NONE   -1

	int colorArray[MAX_COLORS];
	int index_ptr = 0;
	int layout_id = -1;

	float r_mult = 1.0f, g_mult = 1.0f, b_mult = 1.0f;

	// Tune these
	float refRed[3]    = {120, 70, 65};
	float refGreen[3]  = {65, 102, 86};
	float refBlue[3]   = {60, 75, 120};
	float refYellow[3] = {104, 91, 60};

	struct {
		float R_val;
		float G_val;
		float B_val;
		int detected_id;
	} debug;

	struct {
		float R;
		float G;
		float B;
	} debug_notnormalized;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
	float calculate_distance(float current[3], float target[3]);
	void write8(uint8_t reg, uint8_t value);
	uint16_t read16(uint8_t reg);
	void run_calibration(void);
	int get_color_id(void);
	void leds_off();
	void show_color(int color);
	void blink_success();
	void blink_error();
	void wait_all_buttons_release();
	int get_layout_id(int scanned_colors[], int length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	  write8(TCS34725_ATIME, 0xEB);
	  write8(TCS34725_CONTROL, 0x01);

	  write8(TCS34725_ENABLE, 0x01);
	  HAL_Delay(3);
	  write8(TCS34725_ENABLE, 0x03);
	  HAL_Delay(100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  while (1)
	  {
    /* USER CODE END WHILE */
		  int b0 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
		  int b1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
		  // CALIBRATION
		  if (b0 && b1) {
			  HAL_Delay(2000);

			  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) &&
					  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
				  run_calibration();
				  blink_success();
				  leds_off();

				  wait_all_buttons_release();
			  }
		  }

		  // STORE
		  else if (b0) {

			  HAL_Delay(50);  // debounce

			  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)) {
				  if (index_ptr >= MAX_COLORS) {
					  blink_error();
					  leds_off();
					  wait_all_buttons_release();
					  continue;
				  }

				  int color = get_color_id();
				  show_color(color);
				  HAL_Delay(300);

				  if (color != COLOR_NONE) {
					  colorArray[index_ptr++] = color;
					  blink_success();
				  }
				  else {
					  blink_error();
				  }

				  wait_all_buttons_release();
			  }
		  }

		  // UNDO
		  else if (b1) {

			  HAL_Delay(50);  // debounce
			  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {

				  if (index_ptr > 0) {
					  index_ptr--;
					  blink_success();   // blink
					  leds_off();        // then turn off

				  } else {
					  blink_error();
					  leds_off();
				  }

				  wait_all_buttons_release();
			  }
		  }

		  HAL_Delay(50);

		  if (index_ptr == 8) {
		      layout_id = get_layout_id(colorArray, index_ptr);
		  } else {
		      layout_id = -1; // Keep it invalid until the sequence is complete
		  }
    /* USER CODE BEGIN 3 */
	  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV8;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

	void wait_all_buttons_release() {
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) ||
			   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
			HAL_Delay(10);
		}
	}

	void leds_off() {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
	}

	void show_color(int color) {
		leds_off();

		switch(color) {
			case COLOR_RED:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
				break;

			case COLOR_GREEN:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
				break;

			case COLOR_YELLOW:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
				break;

			case COLOR_BLUE:
				for(int i=0;i<3;i++){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
					HAL_Delay(200);

					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
					HAL_Delay(200);

					leds_off();
				}
				break;
		}
	}

	void blink_success() {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
		HAL_Delay(150);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
	}

	void blink_error() {
		for(int i=0;i<3;i++){
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
			HAL_Delay(100);
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
			HAL_Delay(100);
		}
	}

	void run_calibration(void) {
		uint32_t sR=0, sG=0, sB=0;

		for(int i=0;i<SAMPLES;i++){
			sR += read16(TCS34725_RDATAL);
			sG += read16(TCS34725_GDATAL);
			sB += read16(TCS34725_BDATAL);
			HAL_Delay(20);
		}

		float avgR = sR / (float)SAMPLES;
		float avgG = sG / (float)SAMPLES;
		float avgB = sB / (float)SAMPLES;

		float avg = (avgR + avgG + avgB) / 3.0f;

		if (avgR > 0 && avgG > 0 && avgB > 0) {
			r_mult = avg / avgR;
			g_mult = avg / avgG;
			b_mult = avg / avgB;
		}
	}

	float calculate_distance(float current[3], float target[3]) {
		float dr = current[0] - target[0];
		float dg = current[1] - target[1];
		float db = current[2] - target[2];

		return sqrtf(2*dr*dr + 4*dg*dg + 3*db*db);
	}

	int get_color_id(void) {
		uint32_t sC=0, sR=0, sG=0, sB=0;

		for(int i=0;i<SAMPLES;i++){
			sC += read16(TCS34725_CDATAL);
			sR += read16(TCS34725_RDATAL);
			sG += read16(TCS34725_GDATAL);
			sB += read16(TCS34725_BDATAL);
			HAL_Delay(10);
		}

		float avgC = sC / (float)SAMPLES;

		if (avgC < 10) {
		    debug.detected_id = COLOR_NONE;
		    return COLOR_NONE;
		}

		debug_notnormalized.R = sR / (float)SAMPLES;
		debug_notnormalized.G = sG / (float)SAMPLES;
		debug_notnormalized.B = sB / (float)SAMPLES;


		debug.R_val = debug_notnormalized.R * r_mult;
		debug.G_val = debug_notnormalized.G * g_mult;
		debug.B_val = debug_notnormalized.B * b_mult;

		float sum = debug.R_val + debug.G_val + debug.B_val;
		if (sum == 0) {
			debug.detected_id = COLOR_NONE;
			return COLOR_NONE;
		}

		debug.R_val = (debug.R_val / sum) * 255.0f;
		debug.G_val = (debug.G_val / sum) * 255.0f;
		debug.B_val = (debug.B_val / sum) * 255.0f;

		float cur[3] = {debug.R_val, debug.G_val, debug.B_val};

		float dRed    = calculate_distance(cur, refRed);
		float dGreen  = calculate_distance(cur, refGreen);
		float dYellow = calculate_distance(cur, refYellow);
		float dBlue   = calculate_distance(cur, refBlue);

		float minD = dRed;
		int id = COLOR_RED;

		if (dGreen < minD)    { minD = dGreen; id = COLOR_GREEN; }
		if (dYellow < minD) { minD = dYellow; id = COLOR_YELLOW; }
		if (dBlue < minD)   { minD = dBlue; id = COLOR_BLUE; }

		debug.detected_id = id;

		return id;
	}

	int get_layout_id(int scanned_colors[], int length) {
	    if (length < 8) return -1;

	    for (uint16_t i = 0; i < g_layout_count; i++) {
	        int mismatch = 0;
	        for (int branch = 0; branch < 4; branch++) {
	            if (g_layouts[i].slots[branch][0] != scanned_colors[branch * 2] ||
	                g_layouts[i].slots[branch][1] != scanned_colors[branch * 2 + 1]) {
	                mismatch = 1;
	                break;
	            }
	        }
	        if (!mismatch) return i;
	    }
	    return -1;
	}

	void write8(uint8_t reg, uint8_t value) {
		uint8_t tx[2] = {TCS34725_COMMAND_BIT | reg, value};
		HAL_I2C_Master_Transmit(&hi2c1, TCS34725_ADDR, tx, 2, 100);
	}

	uint16_t read16(uint8_t reg) {
		uint8_t buf[2];
		uint8_t cmd = TCS34725_COMMAND_BIT | TCS34725_AUTO_INC | reg;

		HAL_I2C_Master_Transmit(&hi2c1, TCS34725_ADDR, &cmd, 1, 100);
		HAL_I2C_Master_Receive(&hi2c1, TCS34725_ADDR, buf, 2, 100);

		return (buf[1] << 8) | buf[0];
	}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	  /* User can add his own implementation to report the HAL error return state */
	  __disable_irq();
	  while (1)
	  {
	  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	  /* User can add his own implementation to report the file name and line number,
		 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
