#include "main.h"
#include "i2c-lcd.h"
#include "dwt_stm32_delay.h"

#define		led_pin		GPIO_PIN_12
#define		led_out		GPIOB

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);

uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t sum, RH, TEMP;
uint8_t check = 0;
uint16_t nhiet = 0;

/******************************************/

//DHT11
void set_gpio_output (void)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   /*Configure GPIO pin output: PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void set_gpio_input (void)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   /*Configure GPIO pin input: PA10*/
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void DHT11_start (void)
{
   set_gpio_output ();  // set the pin as output
	  // pull the pin low
   HAL_GPIO_WritePin (GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
   DWT_Delay_us (18000);   // wait for 18s
   set_gpio_input ();   // set as input
}
void check_response (void)
{
   DWT_Delay_us (40);
   if (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))
		 // if the pin is low
   {
      DWT_Delay_us (80);  // wait for 80us
      if ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0))) check = 1; 
			// now if the pin is high response = ok i.e. check =1
   }
   while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0))); 
		// wait for the pin to go low
}

uint8_t read_data (void)
{
   uint8_t i,j;
   for (j=0;j<8;j++)
   { 
      while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)));
		 // wait for the pin to go high
      DWT_Delay_us (30);   // wait for 40 us
      if ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)) == 0)  
				// if the pin is low 
      {
         i&= ~(1<<(7-j));   // write 0
      }
      else i|= (1<<(7-j));  // if the pin is high, write 1
      while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)));
			// wait for the pin to go low
   }
   return i;
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
	DWT_Delay_Init ();
	lcd_init ();
	lcd_goto_XY(1,0);
  lcd_send_string("NHIET DO:");
	lcd_goto_XY(2,0);
  lcd_send_string("DO AM   :");
	HAL_GPIO_WritePin(led_out,led_pin,1);
	HAL_Delay(2000);
  while (1)
  {
		DHT11_start ();
		check_response ();
		Rh_byte1 = read_data ();
		Rh_byte2 = read_data ();
		Temp_byte1 = read_data ();
		Temp_byte2 = read_data ();
		sum = read_data();
		
		
		
		if (sum==(Rh_byte1+Rh_byte2+Temp_byte1+Temp_byte2))    
			// if the data is correct
		{
			TEMP = Temp_byte1;
			RH = Rh_byte1;
			lcd_goto_XY(1,0);
			lcd_send_string("NHIET DO:");
			lcd_send_data(TEMP/10%10+48);
			lcd_send_data(TEMP%10+48);
			lcd_send_data(0xdf);
			lcd_send_string("C   ");
		 
			lcd_goto_XY(2,0);
			lcd_send_string("DO AM   :");
			lcd_send_data(RH/10%10+48);
			lcd_send_data(RH%10+48);
			lcd_send_string("%");
		}
		
		if(TEMP>40)	
		{			
			HAL_GPIO_WritePin(led_out,led_pin,1);
			//in canh bao ra man hinh
			lcd_goto_XY(1,0);
			lcd_send_string("NHIET DO QUA CAO!");
		 
			lcd_goto_XY(2,0);
			lcd_send_string("WARNING!!!");
			lcd_send_data(TEMP/10%10+48);
			lcd_send_data(TEMP%10+48);
			lcd_send_data(0xdf);
			lcd_send_string("C   ");
		}
		else      			HAL_GPIO_WritePin(led_out,led_pin,0);
		HAL_Delay(2000);
	}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

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
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
