#include <pack_can_functions.h>
#include <read_sensors.h>
#include <startup_dxl.h>
#include "fdcan.h"
#include "math_ops.h"
#include <stdio.h>
#include <stdint.h>
#include "crc.h"
#include "string.h"
#include "math.h"
#include "ForceSensor.h"


#define VERSION_NUMBER 1.20f


// Initialize CAN FD
FDCAN_RxHeaderTypeDef rxMsg_sense;
FDCAN_TxHeaderTypeDef txHeader_fd_sens;
FDCAN_FilterTypeDef sense_can_filt;
uint8_t txMsg_fd_sens[48]; // unused, previously used to send can messages over pcan

uint8_t sensor_rx_buf[8]; // receives can messages
uint8_t elapsedTime_buf[4]; // buffer with time to transmit over uart
uint8_t eol[4] = {0xFF, 0xFF, 0xFF, 0xFF};


// Variables for force sensor data
int32_t pressure_raw[8]; // unused rn
uint32_t elapsedTime = 0; // unused rn

uint8_t data_buffer[32]; // buffer with pressure values to transmit over uart


// main CPP loop
int dxl_read_main(void)
{
	printf("\r\n--------MIT Read Sensor Vals--------\r\n");
	printf("Version No: %.2f\r\n\n\n", VERSION_NUMBER);

	//Rx Filters
	sense_can_filt.IdType = FDCAN_STANDARD_ID;
	sense_can_filt.FilterIndex = 0;
	sense_can_filt.FilterType = FDCAN_FILTER_RANGE;
	sense_can_filt.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
	sense_can_filt.FilterID1 = 0x00;
	sense_can_filt.FilterID2 = 0x0E; // up to 0x0E for phalange sensors (14)
	sense_can_filt.RxBufferIndex = 0;

	// CAN Config
	if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sense_can_filt) != HAL_OK)
	{
		printf("Error in filter config. CAN FD2 \n\r");
		Error_Handler();
	}

	//Initialize CAN Bus
	if ((HAL_FDCAN_Start(&hfdcan2)) != HAL_OK )
	{
		printf("Failed to start sensor CAN.\n\r");
		while(1);
	}


	// enable CAN Interrupts
	HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_RX_FIFO1_NEW_MESSAGE,0);// Initialize CAN2 Rx1 Interrupt


	while (1) {


//		// print for now
//		printf("Pressure data: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n\r", pressure_raw[0],
//				pressure_raw[1],
//				pressure_raw[2],
//				pressure_raw[3],
//				pressure_raw[4],
//				pressure_raw[5],
//				pressure_raw[6],
//				pressure_raw[7]);
//		printf("Time: %ld\n\r", elapsedTime);
		// Send data buffer
	    HAL_UART_Transmit(&huart6, data_buffer, 32, HAL_MAX_DELAY);
//
//
//		 Transmit time buffer
		HAL_UART_Transmit(&huart6, elapsedTime_buf, 4, HAL_MAX_DELAY);

//		 Transmit eol buffer
		HAL_UART_Transmit(&huart6, eol, 4, HAL_MAX_DELAY);  // This was 779



}
}




// unpack raw pressure sensor values and eval_time
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *canHandle, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET){
			HAL_FDCAN_GetRxMessage(canHandle, FDCAN_RX_FIFO1, &rxMsg_sense, sensor_rx_buf);
			uint8_t id = rxMsg_sense.Identifier;


			//unpack pressure sensor values
			if (id == 0){
//				uint32_t p_raw_1 = (sensor_rx_buf[0] << 24) | (sensor_rx_buf[1] << 16) | (sensor_rx_buf[2] << 8) | sensor_rx_buf[3];
//				uint32_t p_raw_2 = (sensor_rx_buf[4] << 24) | (sensor_rx_buf[5] << 16) | (sensor_rx_buf[6] << 8) | sensor_rx_buf[7];
//				pressure_raw[0] = p_raw_1;
//				pressure_raw[1]  = p_raw_2;
				for (int i = 0; i < 8; ++i){
					data_buffer[i] = sensor_rx_buf[i];
				}


				}
			else if (id == 1){
//				uint32_t p_raw_1 = (sensor_rx_buf[0] << 24) | (sensor_rx_buf[1] << 16) | (sensor_rx_buf[2] << 8) | sensor_rx_buf[3];
//				uint32_t p_raw_2 = (sensor_rx_buf[4] << 24) | (sensor_rx_buf[5] << 16) | (sensor_rx_buf[6] << 8) | sensor_rx_buf[7];
//				pressure_raw[2] = p_raw_1;
//				pressure_raw[3]  = p_raw_2;
				for (int i = 0; i < 8; ++i){
					data_buffer[i+8] = sensor_rx_buf[i];
				}

			}
			else if (id == 2){
//				uint32_t p_raw_1 = (sensor_rx_buf[0] << 24) | (sensor_rx_buf[1] << 16) | (sensor_rx_buf[2] << 8) | sensor_rx_buf[3];
//				uint32_t p_raw_2 = (sensor_rx_buf[4] << 24) | (sensor_rx_buf[5] << 16) | (sensor_rx_buf[6] << 8) | sensor_rx_buf[7];
//				pressure_raw[4] = p_raw_1;
//				pressure_raw[5]  = p_raw_2;
				for (int i = 0; i < 8; ++i){
					data_buffer[i+16] = sensor_rx_buf[i];
				}

				}
			else if (id == 3){
//				uint32_t p_raw_1 = (sensor_rx_buf[0] << 24) | (sensor_rx_buf[1] << 16) | (sensor_rx_buf[2] << 8) | sensor_rx_buf[3];
//				uint32_t p_raw_2 = (sensor_rx_buf[4] << 24) | (sensor_rx_buf[5] << 16) | (sensor_rx_buf[6] << 8) | sensor_rx_buf[7];
//				pressure_raw[6] = p_raw_1;
//				pressure_raw[7]  = p_raw_2;
				for (int i = 0; i < 8; ++i){
					data_buffer[i+24] = sensor_rx_buf[i];
				}

				}

			else if (id == 4){
//				uint32_t eval_time = (sensor_rx_buf[0] << 24) | (sensor_rx_buf[1] << 16) | (sensor_rx_buf[2] << 8) | sensor_rx_buf[3];
//				elapsedTime = eval_time;
				for (int i = 0; i < 4; ++i){
					elapsedTime_buf[i] = sensor_rx_buf[i];
				}
			}


		}

}
