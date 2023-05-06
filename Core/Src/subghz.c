/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz.c
  * @brief   This file provides code for the configuration
  *          of the SUBGHZ instances.
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
#include "subghz.h"

/* USER CODE BEGIN 0 */
extern char uartBuff[1024];
extern uint8_t rxBuffer[RX_SIZE];
extern uint8_t rxBuffer_encypt[RX_SIZE];
extern uint8_t rxBuffer_decypt[RX_SIZE];
extern uint8_t size;
volatile uint32_t RF_FREQUENCY= 433000000;
pingPongFSM_t fsm;
PacketParams_t packetParams;  // TODO: this is lazy
const RadioLoRaBandwidths_t Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 };
void (*volatile eventReceptor)(pingPongFSM_t *const fsm);
void RadioOnDioIrq(RadioIrqMasks_t radioIrq);


/* USER CODE END 0 */

SUBGHZ_HandleTypeDef hsubghz;

/* SUBGHZ init function */
void MX_SUBGHZ_Init(void)
{

  /* USER CODE BEGIN SUBGHZ_Init 0 */

  /* USER CODE END SUBGHZ_Init 0 */

  /* USER CODE BEGIN SUBGHZ_Init 1 */

  /* USER CODE END SUBGHZ_Init 1 */
  hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
  if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SUBGHZ_Init 2 */

  /* USER CODE END SUBGHZ_Init 2 */

}

void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef* subghzHandle)
{

  /* USER CODE BEGIN SUBGHZ_MspInit 0 */

  /* USER CODE END SUBGHZ_MspInit 0 */
    /* SUBGHZ clock enable */
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();

    /* SUBGHZ interrupt Init */
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
  /* USER CODE BEGIN SUBGHZ_MspInit 1 */

  /* USER CODE END SUBGHZ_MspInit 1 */
}

void HAL_SUBGHZ_MspDeInit(SUBGHZ_HandleTypeDef* subghzHandle)
{

  /* USER CODE BEGIN SUBGHZ_MspDeInit 0 */

  /* USER CODE END SUBGHZ_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SUBGHZSPI_CLK_DISABLE();

    /* SUBGHZ interrupt Deinit */
    HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
  /* USER CODE BEGIN SUBGHZ_MspDeInit 1 */

  /* USER CODE END SUBGHZ_MspDeInit 1 */
}

/* USER CODE BEGIN 1 */
void radioInit(void)
{
  // Initialize the hardware (SPI bus, TCXO control, RF switch)
  SUBGRF_Init(RadioOnDioIrq);

  // Use DCDC converter if `DCDC_ENABLE` is defined in radio_conf.h
  // "By default, the SMPS clock detection is disabled and must be enabled before enabling the SMPS." (6.1 in RM0453)
  SUBGRF_WriteRegister(SUBGHZ_SMPSC0R, (SUBGRF_ReadRegister(SUBGHZ_SMPSC0R) | SMPS_CLK_DET_ENABLE));
  SUBGRF_SetRegulatorMode();

  // Use the whole 256-byte buffer for both TX and RX
  SUBGRF_SetBufferBaseAddress(0x00, 0x00);

  SUBGRF_SetRfFrequency(RF_FREQUENCY);
  SUBGRF_SetRfTxPower(TX_OUTPUT_POWER);
  SUBGRF_SetStopRxTimerOnPreambleDetect(false);

  SUBGRF_SetPacketType(PACKET_TYPE_LORA);

  SUBGRF_WriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF );
  SUBGRF_WriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF );

  ModulationParams_t modulationParams;
  modulationParams.PacketType = PACKET_TYPE_LORA;
  modulationParams.Params.LoRa.Bandwidth = Bandwidths[LORA_BANDWIDTH];
  modulationParams.Params.LoRa.CodingRate = (RadioLoRaCodingRates_t)LORA_CODINGRATE;
  modulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
  modulationParams.Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)LORA_SPREADING_FACTOR;
  SUBGRF_SetModulationParams(&modulationParams);

  packetParams.PacketType = PACKET_TYPE_LORA;
  packetParams.Params.LoRa.CrcMode = LORA_CRC_ON;
  packetParams.Params.LoRa.HeaderType = LORA_PACKET_VARIABLE_LENGTH;
  packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;
  packetParams.Params.LoRa.PayloadLength = 0xFF;


  packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH;
  SUBGRF_SetPacketParams(&packetParams);

  //SUBGRF_SetLoRaSymbNumTimeout(LORA_SYMBOL_TIMEOUT);

  // WORKAROUND - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
  // RegIqPolaritySetup @address 0x0736
  SUBGRF_WriteRegister( 0x0736, SUBGRF_ReadRegister( 0x0736 ) | ( 1 << 2 ) );
}
void RadioOnDioIrq(RadioIrqMasks_t radioIrq)
{
  switch (radioIrq)
  {
    case IRQ_TX_DONE:
      eventReceptor = eventTxDone;
      break;
    case IRQ_RX_DONE:
      eventReceptor = eventRxDone;
      break;
    case IRQ_RX_TX_TIMEOUT:
      if (SUBGRF_GetOperatingMode() == MODE_TX)
      {
        eventReceptor = eventTxTimeout;
      }
      else if (SUBGRF_GetOperatingMode() == MODE_RX)
      {
        eventReceptor = eventRxTimeout;
      }
      break;
    case IRQ_CRC_ERROR:
      eventReceptor = eventRxError;
      break;
    default:
      break;
  }
}


/**
  * @brief  Process the TX Done event
  * @param  fsm pointer to FSM context
  * @retval None
  */
void eventTxDone(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Event TX Done\r\n", 15, HAL_MAX_DELAY);
  switch (fsm->state)
  {
    case STATE_MASTER:
      switch (fsm->subState)
      {
        case SSTATE_TX:
          enterMasterRx(fsm);
          fsm->subState = SSTATE_RX;
          break;
        default:
          break;
      }
      break;
    case STATE_SLAVE:
      switch (fsm->subState)
      {
        case SSTATE_TX:
          enterSlaveRx(fsm);
          fsm->subState = SSTATE_RX;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


/**
  * @brief  Process the RX Done event
  * @param  fsm pointer to FSM context
  * @retval None
  */
void eventRxDone(pingPongFSM_t *const fsm)
{
  char DataREC[1023];
  HAL_UART_Transmit(&huart2, (uint8_t *)"Event RX Done\r\n", 15, HAL_MAX_DELAY);
  switch(fsm->state)
  {
    case STATE_MASTER:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          transitionRxDone(fsm);
          memcpy(rxBuffer_encypt,fsm->rxBuffer,sizeof(fsm->rxBuffer));
          if (strncmp(fsm->rxBuffer, "PONG", 4) == 0)
          {
        	sprintf(DataREC, "\n\rData rec: %s\r\n",fsm->rxBuffer);
        	HAL_UART_Transmit(&huart2, (uint8_t *)DataREC, strlen(DataREC), HAL_MAX_DELAY);
        	 BSP_LED_Toggle(LED_GREEN);
            enterMasterTx(fsm);
            fsm->subState = SSTATE_TX;
          }
          else if (strncmp(fsm->rxBuffer, "PING", 4) == 0)
          {
            enterSlaveRx(fsm);
            fsm->state = STATE_SLAVE;
          }
          else
          {

            enterMasterRx(fsm);
          }
          break;
        default:
          break;
      }
      break;
    case STATE_SLAVE:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          transitionRxDone(fsm);
          if (strncmp(fsm->rxBuffer, "PING", 4) == 0)
          {
           // BSP_LED_Off(LED_RED);
           // BSP_LED_Toggle(LED_GREEN);
        	  BSP_LED_Toggle(LED_GREEN);
            enterSlaveTx(fsm);
            fsm->subState = SSTATE_TX;
          }
          else
          {
            enterMasterRx(fsm);
            fsm->state = STATE_MASTER;
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


/**
  * @brief  Process the TX Timeout event
  * @param  fsm pointer to FSM context
  * @retval None
  */
void eventTxTimeout(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Event TX Timeout\r\n", 18, HAL_MAX_DELAY);
  switch (fsm->state)
  {
    case STATE_MASTER:
      switch (fsm->subState)
      {
        case SSTATE_TX:
          enterMasterRx(fsm);
          fsm->subState = SSTATE_RX;
          break;
        default:
          break;
      }
      break;
    case STATE_SLAVE:
      switch (fsm->subState)
      {
        case SSTATE_TX:
          enterSlaveRx(fsm);
          fsm->subState = SSTATE_RX;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


/**
  * @brief  Process the RX Timeout event
  * @param  fsm pointer to FSM context
  * @retval None
  */
void eventRxTimeout(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Event RX Timeout\r\n", 18, HAL_MAX_DELAY);
  switch (fsm->state)
  {
    case STATE_MASTER:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          HAL_Delay(fsm->randomDelay);
          enterMasterTx(fsm);
          fsm->subState = SSTATE_TX;
          break;
        default:
          break;
      }
      break;
    case STATE_SLAVE:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          enterSlaveRx(fsm);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


/**
  * @brief  Process the RX Error event
  * @param  fsm pointer to FSM context
  * @retval None
  */
void eventRxError(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Event Rx Error\r\n", 16, HAL_MAX_DELAY);
  switch (fsm->state)
  {
    case STATE_MASTER:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          HAL_Delay(fsm->randomDelay);
          enterMasterTx(fsm);
          fsm->subState = SSTATE_TX;
          break;
        default:
          break;
      }
      break;
    case STATE_SLAVE:
      switch (fsm->subState)
      {
        case SSTATE_RX:
          enterSlaveRx(fsm);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


/**
  * @brief  Entry actions for the RX sub-state of the Master state
  * @param  fsm pointer to FSM context
  * @retval None
  */
void enterMasterRx(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Master Rx start\r\n", 17, HAL_MAX_DELAY);
  SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE );
  SUBGRF_SetSwitch(RFO_LP, RFSWITCH_RX);
  packetParams.Params.LoRa.PayloadLength = 0xFF;
  SUBGRF_SetPacketParams(&packetParams);
  SUBGRF_SetRx(fsm->rxTimeout << 6);
}


/**
  * @brief  Entry actions for the RX sub-state of the Slave state
  * @param  fsm pointer to FSM context
  * @retval None
  */
void enterSlaveRx(pingPongFSM_t *const fsm)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"Slave Rx start\r\n", 16, HAL_MAX_DELAY);
  SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE );
  SUBGRF_SetSwitch(RFO_LP, RFSWITCH_RX);
  packetParams.Params.LoRa.PayloadLength = 0xFF;
  SUBGRF_SetPacketParams(&packetParams);
  SUBGRF_SetRx(fsm->rxTimeout << 6);
}


/**
  * @brief  Entry actions for the TX sub-state of the Master state
  * @param  fsm pointer to FSM context
  * @retval None
  */
void enterMasterTx(pingPongFSM_t *const fsm)
{
  HAL_Delay(fsm->rxMargin);

  HAL_UART_Transmit(&huart2, (uint8_t *)"...PING\r\n", 9, HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t *)"Master Tx start\r\n", 17, HAL_MAX_DELAY);
  SUBGRF_SetDioIrqParams( IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                          IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE );
  SUBGRF_SetSwitch(RFO_LP, RFSWITCH_TX);
  // Workaround 5.1 in DS.SX1261-2.W.APP (before each packet transmission)
  SUBGRF_WriteRegister(0x0889, (SUBGRF_ReadRegister(0x0889) | 255));
  packetParams.Params.LoRa.PayloadLength = 0xFF;
  SUBGRF_SetPacketParams(&packetParams);
  SUBGRF_SendPayload((uint8_t *)"PINGpppppppppppppppppppppppppppppppppppppppp", 255, 0);
}


/**
  * @brief  Entry actions for the TX sub-state of the Slave state
  * @param  fsm pointer to FSM context
  * @retval None
  */
void enterSlaveTx(pingPongFSM_t *const fsm)
{
  HAL_Delay(fsm->rxMargin);

  HAL_UART_Transmit(&huart2, (uint8_t *)"...PONG\r\n", 9, HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t *)"Slave Tx start\r\n", 16, HAL_MAX_DELAY);
  SUBGRF_SetDioIrqParams( IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                          IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE );
  SUBGRF_SetSwitch(RFO_LP, RFSWITCH_TX);
  // Workaround 5.1 in DS.SX1261-2.W.APP (before each packet transmission)
  SUBGRF_WriteRegister(0x0889, (SUBGRF_ReadRegister(0x0889) | 0xFF));
  packetParams.Params.LoRa.PayloadLength = 0xFF;
  SUBGRF_SetPacketParams(&packetParams);
  //SUBGRF_SendPayload((uint8_t *)"PONGdsaaahhhhhhaaaaamnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\r\n", 255, 0);
  //SUBGRF_SendPayload(rxBuffer, 255, 0);
  //memset(rxBuffer,' ',sizeof(rxBuffer));
   SUBGRF_SendPayload(rxBuffer_encypt, 255, 0);
   //memset(rxBuffer_encypt,' ',sizeof(rxBuffer_encypt));
  //strcpy(rxBuffer,"PONGnoLoad");
  memset(rxBuffer,0,sizeof(rxBuffer));
}


/**
  * @brief  Transition actions executed on every RX Done event (helper function)
  * @param  fsm pointer to FSM context
  * @retval None
  */
void transitionRxDone(pingPongFSM_t *const fsm)
{
  PacketStatus_t packetStatus;
  char uartBuff[50];

  // Workaround 15.3 in DS.SX1261-2.W.APP (because following RX w/ timeout sequence)
  SUBGRF_WriteRegister(0x0920, 0x00);
  SUBGRF_WriteRegister(0x0944, (SUBGRF_ReadRegister(0x0944) | 0x02));

  SUBGRF_GetPayload((uint8_t *)fsm->rxBuffer, &fsm->rxSize, 0xFF);
  SUBGRF_GetPacketStatus(&packetStatus);

  sprintf(uartBuff, "RssiValue=%d dBm, SnrValue=%d Hz\r\n", packetStatus.Params.LoRa.RssiPkt, packetStatus.Params.LoRa.SnrPkt);
  HAL_UART_Transmit(&huart2, (uint8_t *)uartBuff, strlen(uartBuff), HAL_MAX_DELAY);
}
void fsmInit(void)
{

	// get random number
	  uint32_t rnd = 0;
	  SUBGRF_SetDioIrqParams(IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
	  rnd = SUBGRF_GetRandom();

	  fsm.state = STATE_NULL;
	  fsm.subState = SSTATE_NULL;
	  fsm.rxTimeout = 3000; // 3000 ms
	  fsm.rxMargin = 200;   // 200 ms
	  fsm.randomDelay = rnd >> 22; // [0, 1023] ms
	  //sprintf(uartBuff, "rand=%lu\r\n", fsm.randomDelay);
	  HAL_UART_Transmit(&huart2, (uint8_t *)uartBuff, strlen(uartBuff), HAL_MAX_DELAY);

	  HAL_Delay(fsm.randomDelay);
	  SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR,
	                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR,
	                          IRQ_RADIO_NONE,
	                          IRQ_RADIO_NONE );
	  SUBGRF_SetSwitch(RFO_LP, RFSWITCH_RX);
	  SUBGRF_SetRx(fsm.rxTimeout << 6);
	  fsm.state = STATE_SLAVE;
	  //fsm.subState = SSTATE_RX;
	  //fsm.state = STATE_MASTER;
	  fsm.subState = SSTATE_RX;
}
void RFGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  // Enable GPIO Clocks
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  // DEBUG_SUBGHZSPI_{NSSOUT, SCKOUT, MSIOOUT, MOSIOUT} pins
	  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_SUBGHZSPI;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  // DEBUG_RF_{HSE32RDY, NRESET} pins
	  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  // DEBUG_RF_{SMPSRDY, LDORDY} pins
	  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  // RF_BUSY pin
	  GPIO_InitStruct.Pin = GPIO_PIN_12;
	  GPIO_InitStruct.Alternate = GPIO_AF6_RF_BUSY;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  // RF_{IRQ0, IRQ1, IRQ2} pins
	  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_8;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
void SUBGHZ(void)
{
	    eventReceptor = NULL;
	    while (eventReceptor == NULL);
	    {
	    radioInit();
	    eventReceptor(&fsm);
	    }
}

/* USER CODE END 1 */
