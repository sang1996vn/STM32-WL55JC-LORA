/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    aes.c
  * @brief   This file provides code for the configuration
  *          of the AES instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "aes.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#define MAX_SUBSTRING_LENGTH 32

#define BLOCK_SIZE 32 // block size 128-bit
//#define MAX_DATA_SIZE 1024

extern uint8_t rxBuffer[RX_SIZE];
extern uint8_t rxBuffer_encypt[RX_SIZE];
extern uint8_t rxBuffer_decypt[RX_SIZE];
extern uint8_t size;

#if defined(__GNUC__)
int _write(int fd, char * ptr, int len) {
  HAL_UART_Transmit( & huart2, (uint8_t * ) ptr, len, HAL_MAX_DELAY);
  return len;
}
#elif defined(__ICCARM__)#include "LowLevelIOInterface.h"

size_t __write(int handle,
  const unsigned char * buffer, size_t size) {
  HAL_UART_Transmit( & huart2, (uint8_t * ) buffer, size, HAL_MAX_DELAY);
  return size;
}
#elif defined(__CC_ARM)
int fputc(int ch, FILE * f) {
  HAL_UART_Transmit( & huart2, (uint8_t * ) & ch, 1, HAL_MAX_DELAY);
  return ch;
}
#endif




/* USER CODE END 0 */

CRYP_HandleTypeDef hcryp;
__ALIGN_BEGIN static const uint32_t pKeyAES[8] __ALIGN_END = {
                            0x603DEB10,0x15CA71BE,0x2B73AEF0,0x857D7781,0x1F352C07,0x3B6108D7,0x2D9810A3,0x0914DFF4};

/* AES init function */
void MX_AES_Init(void)
{

  /* USER CODE BEGIN AES_Init 0 */

  /* USER CODE END AES_Init 0 */

  /* USER CODE BEGIN AES_Init 1 */

  /* USER CODE END AES_Init 1 */
  hcryp.Instance = AES;
  hcryp.Init.DataType = CRYP_DATATYPE_1B;
  hcryp.Init.KeySize = CRYP_KEYSIZE_256B;
  hcryp.Init.pKey = (uint32_t *)pKeyAES;
  hcryp.Init.Algorithm = CRYP_AES_ECB;
  hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp.Init.HeaderWidthUnit = CRYP_HEADERWIDTHUNIT_BYTE;
  hcryp.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN AES_Init 2 */

  /* USER CODE END AES_Init 2 */

}

void HAL_CRYP_MspInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES)
  {
  /* USER CODE BEGIN AES_MspInit 0 */

  /* USER CODE END AES_MspInit 0 */
    /* AES clock enable */
    __HAL_RCC_AES_CLK_ENABLE();
  /* USER CODE BEGIN AES_MspInit 1 */

  /* USER CODE END AES_MspInit 1 */
  }
}

void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES)
  {
  /* USER CODE BEGIN AES_MspDeInit 0 */

  /* USER CODE END AES_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_AES_CLK_DISABLE();
  /* USER CODE BEGIN AES_MspDeInit 1 */

  /* USER CODE END AES_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void AES_start(void)
{


	/* Mã hóa dữ liệu */
	if(rxBuffer[0]=='\0')
	{
		memset(rxBuffer_encypt,0,sizeof(rxBuffer_encypt));
	}
	else
		AES_Encrypt(rxBuffer,rxBuffer_encypt,sizeof(rxBuffer));

	printf("Encrypted data: ");
	print_array(rxBuffer_encypt, strlen(rxBuffer_encypt));


	/* Giải mã dữ liệu */


//	if(rxBuffer_encypt[0]=='\0')
//		{
//			memset(rxBuffer_decypt,0,sizeof(rxBuffer_decypt));
//		}
//		else
//			AES_Decrypt(rxBuffer_encypt,rxBuffer_decypt,sizeof(rxBuffer_encypt));
//
//	printf("Decrypted data: %s\n", rxBuffer_decypt);
//	memset(rxBuffer_decypt,0,sizeof(rxBuffer_decypt));
//	memset(rxBuffer_encypt,0,sizeof(rxBuffer_encypt));







}



void print_array(uint8_t *array, uint32_t size) {
    for (int i = 0; i < size; i++) {
        printf("%02x ", array[i]);

    }
    printf("\n");

}
void AES_Encrypt(uint8_t *input_data, uint8_t *output_data, uint32_t data_size) {
	MX_AES_Init();
    uint8_t *input_block = input_data;
    uint8_t *output_block = output_data;
    uint32_t remaining_data = data_size;

    while (remaining_data >= BLOCK_SIZE) {
        if (HAL_CRYP_Encrypt(&hcryp, input_block, BLOCK_SIZE, output_block, 1000) != HAL_OK) {
            // Xử lý lỗi khi mã hóa AES
            return;
        }
        input_block += BLOCK_SIZE;
        output_block += BLOCK_SIZE;
        remaining_data -= BLOCK_SIZE;
    }

    // Mã hóa block cuối cùng
    if (remaining_data > 0) {
        if (HAL_CRYP_Encrypt(&hcryp, input_block, remaining_data, output_block, 1000) != HAL_OK) {
            // Xử lý lỗi khi mã hóa AES
            return;
        }
    }

    HAL_CRYP_DeInit(&hcryp);
}
void AES_Decrypt(uint8_t *input_data, uint8_t *output_data, uint32_t data_size) {

	MX_AES_Init();
    uint8_t *input_block = input_data;
    uint8_t *output_block = output_data;
    uint32_t remaining_data = data_size;

    while (remaining_data >= BLOCK_SIZE) {
        if (HAL_CRYP_Decrypt(&hcryp, input_block, BLOCK_SIZE, output_block, 1000) != HAL_OK) {
            // Xử lý lỗi khi giải mã AES
            return;
        }
        input_block += BLOCK_SIZE;
        output_block += BLOCK_SIZE;
        remaining_data -= BLOCK_SIZE;
    }

    // Giải mã block cuối cùng
    if (remaining_data > 0) {
        if (HAL_CRYP_Decrypt(&hcryp, input_block, remaining_data, output_block, 1000) != HAL_OK) {
            // Xử lý lỗi khi giải mã AES
            return;
        }
    }

    HAL_CRYP_DeInit(&hcryp);
}
static void sim_thread(void *arg)
{
	/* */
	/* Khai bao bien hoac viet cac thu tuc can thiet o day neu co */
	/* */
	CheckSimStatus();
	while(1) {

		osDelay(5000);
	}
}
void sim_thread_init(void)
{
	osThreadCreate(thread_def, argument)("SIM", sim_thread, NULL, SIM_THREAD_STACKSIZE, SIM_THREAD_PRIO);
}
/* USER CODE END 1 */
