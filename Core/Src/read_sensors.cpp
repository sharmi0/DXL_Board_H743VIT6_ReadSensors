#include <pack_can_functions.h>
#include <read_sensors.h>
#include <startup_dxl.h>
#include "fdcan.h"
#include "math_ops.h"
#include <stdio.h>
#include <stdint.h>
#include "crc.h"
#include "string.h"
#include <XM430_bus.h>
#include "math.h"
#include "actuator_transformation.h"
#include "ForceSensor.h"
#include "neural_nets.h"
#define rad2pulse_t(x) uint32_t(rad2pulse(x))
#define deg2rad(x) float((PI/180.0f)*x)
#define pulse2deg(x) (360.0f/4096.0f)*(float)(x-2048.0f)
#define VERSION_NUMBER 1.20f


// Initialize CAN FD
FDCAN_RxHeaderTypeDef rxMsg_sense;
FDCAN_FilterTypeDef sense_can_filt;

uint8_t sense_rx_buf[16]; // TODO: could make this shorter?


// Variables for force sensor data
int32_t pressure_raw1[8];
//int32_t pressure_raw2[8];
uint8_t tof1[8];
//uint8_t tof2[8];

float force1[7];
//float force2[7];

//extern NeuralNet sensorB3;
//ForceSensor forcesensor1(0, &sensorB3);
//extern NeuralNet sensorB4;
//ForceSensor forcesensor2(1, &sensorB4);


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



	if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sense_can_filt) != HAL_OK)
	{
		printf("Error in filter config. CAN FD2 \n\r");
		Error_Handler();
	}


	if ((HAL_FDCAN_Start(&hfdcan2)) != HAL_OK ) //Initialize CAN Bus
	{
		printf("Failed to start sensor CAN.\n\r");
		while(1);
	}


	// enable CAN Interrupts
	HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_RX_FIFO1_NEW_MESSAGE,0);// Initialize CAN2 Rx1 Interrupt


	int loop_count = 0;
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	while (1)
	{

//			printf("loop time: %lu \r\n",eval_time);
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
//			HAL_Delay(100);
			printf("Force data: %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n\r", force1[0],force1[1],force1[2],force1[3],force1[4],force1[5],force1[6], force1[7]);
//			printf("Pressure data: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n\r", pressure_raw1[0],pressure_raw1[1],pressure_raw1[2],pressure_raw1[3],pressure_raw1[4],pressure_raw1[5],pressure_raw1[6],pressure_raw1[7]);
//			printf("TOF1: %03d,%03d,%03d,%03d,%03d\n\r", tof1[0], tof1[1], tof1[2], tof1[3], tof1[4]);
//			printf("TOF2: %03d,%03d,%03d,%03d,%03d\n\r", tof2[0], tof2[1], tof2[2], tof2[3], tof2[4]);

		loop_count++;
	}
}





// new unpacking for neural net values from fingertip sensors and extra ToF
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *canHandle, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET){
			HAL_FDCAN_GetRxMessage(canHandle, FDCAN_RX_FIFO1, &rxMsg_sense, sense_rx_buf);
			uint8_t id = rxMsg_sense.Identifier;

//			printf("%d\n\r", id);

			if (id == CAN2_FORCE_1){
				// unpack forces and angles
				uint8_t fx_int = sense_rx_buf[0];
				uint8_t fy_int = sense_rx_buf[1];
				uint8_t fz_int = sense_rx_buf[2];
				uint8_t fn_int = sense_rx_buf[3];
				uint8_t theta_int = sense_rx_buf[4];
				uint8_t phi_int = sense_rx_buf[5];
				uint8_t contact_flag_int = sense_rx_buf[6];

				/// convert uints to floats ///
				force1[0] = uint_to_float(fx_int, FT_MIN, FT_MAX, 8);
				force1[1]  = uint_to_float(fy_int, FT_MIN, FT_MAX, 8);
				force1[2]  = uint_to_float(fz_int, FN_MIN, FN_MAX, 8);
				force1[3]  = uint_to_float(fn_int, FN_MIN, FN_MAX, 8);
				force1[4]  = uint_to_float(theta_int, ANG_MIN_THETA, ANG_MAX_THETA, 8);
				force1[5]  = uint_to_float(phi_int, ANG_MIN_PHI, ANG_MAX_PHI, 8);
				force1[6]  = uint_to_float(contact_flag_int, CFLAG_MIN, CFLAG_MAX, 8);
				force1[7]  = sense_rx_buf[7];
			}
//			else if (id == CAN2_FORCE_2){
//
//				// unpack forces and angles
//				uint8_t fx_int = sense_rx_buf[0];
//				uint8_t fy_int = sense_rx_buf[1];
//				uint8_t fz_int = sense_rx_buf[2];
//				uint8_t fn_int = sense_rx_buf[3];
//				uint8_t theta_int = sense_rx_buf[4];
//				uint8_t phi_int = sense_rx_buf[5];
//				uint8_t contact_flag_int = sense_rx_buf[6];
//				/// convert uints to floats ///
//				force2[0] = uint_to_float(fx_int, FT_MIN, FT_MAX, 8);
//				force2[1]  = uint_to_float(fy_int, FT_MIN, FT_MAX, 8);
//				force2[2]  = uint_to_float(fz_int, FN_MIN, FN_MAX, 8);
//				force2[3]  = uint_to_float(fn_int, FN_MIN, FN_MAX, 8);
//				force2[4]  = uint_to_float(theta_int, ANG_MIN_THETA, ANG_MAX_THETA, 8);
//				force2[5]  = uint_to_float(phi_int, ANG_MIN_PHI, ANG_MAX_PHI, 8);
//				force2[6]  = uint_to_float(contact_flag_int, CFLAG_MIN, CFLAG_MAX, 8);
//			}
			else if (id == CAN2_TOF_1){
				for(int i = 0;i<8;i++){
					tof1[i] = sense_rx_buf[i];
				}
			}
//			else if (id == CAN2_TOF_2){
//				for(int i = 0;i<8;i++){
//					tof2[i] = sense_rx_buf[i];
//				}
//			}
			//unpack pressure sensor values
			else if (id == 0){
				uint32_t p_raw_1 = (sense_rx_buf[0] << 24) | (sense_rx_buf[1] << 16) | (sense_rx_buf[2] << 8) | sense_rx_buf[3];
				uint32_t p_raw_2 = (sense_rx_buf[4] << 24) | (sense_rx_buf[5] << 16) | (sense_rx_buf[6] << 8) | sense_rx_buf[7];
				pressure_raw1[0] = p_raw_1;
				pressure_raw1[1]  = p_raw_2;

				}
			else if (id == 1){
				uint32_t p_raw_1 = (sense_rx_buf[0] << 24) | (sense_rx_buf[1] << 16) | (sense_rx_buf[2] << 8) | sense_rx_buf[3];
				uint32_t p_raw_2 = (sense_rx_buf[4] << 24) | (sense_rx_buf[5] << 16) | (sense_rx_buf[6] << 8) | sense_rx_buf[7];
				pressure_raw1[2] = p_raw_1;
				pressure_raw1[3]  = p_raw_2;

			}
			else if (id == 2){
				uint32_t p_raw_1 = (sense_rx_buf[0] << 24) | (sense_rx_buf[1] << 16) | (sense_rx_buf[2] << 8) | sense_rx_buf[3];
				uint32_t p_raw_2 = (sense_rx_buf[4] << 24) | (sense_rx_buf[5] << 16) | (sense_rx_buf[6] << 8) | sense_rx_buf[7];
				pressure_raw1[4] = p_raw_1;
				pressure_raw1[5]  = p_raw_2;

				}
			else if (id == 3){
				uint32_t p_raw_1 = (sense_rx_buf[0] << 24) | (sense_rx_buf[1] << 16) | (sense_rx_buf[2] << 8) | sense_rx_buf[3];
				uint32_t p_raw_2 = (sense_rx_buf[4] << 24) | (sense_rx_buf[5] << 16) | (sense_rx_buf[6] << 8) | sense_rx_buf[7];
				pressure_raw1[6] = p_raw_1;
				pressure_raw1[7]  = p_raw_2;

				}


		}

}
