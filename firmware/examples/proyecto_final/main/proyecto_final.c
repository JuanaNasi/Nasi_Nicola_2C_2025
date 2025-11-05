/*! @mainpage Proyecto Final
 *
 * @section Monitor de postura
 *
 * This section describes how the program works.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 22/10/2025 | Creación del documento	                     |
 * | 29/10/2025 | Terminación de cálculo de ángulos	             |
 * | 04/11/2025 | Cambio de VTaskDelay por Timers	             |
 *
 * @author Juana Nasi (juananasi3009@gmail.com)
 * @author Josefina Nicola (josefina.nicola@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <math.h>
#include "ADXL335.h"
#include "analog_io_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

#define MEDICION_PERIODO_US 10000
#define MUESTREO_PERIODO_US 1000000
#define n 4 

/*==================[internal data declaration]==============================*/

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

float CalcularPitch(float x, float y, float z);
float CalcularRoll(float x, float y, float z);


/*==================[internal data definition]===============================*/

bool flag_mostrar = false;
float x_prom;
float y_prom; 
float z_prom;


/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/


void FuncTimer(void *param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

void Medir(void *param){
	while(1){
		x_prom = 0;
		y_prom = 0;
		z_prom = 0;
		
		//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		for (int i = 0; i < n; i++){
			x_prom += ReadXValue()/n;
			y_prom += ReadYValue()/n;
			z_prom += ReadZValue()/n;
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
	}
	
	
}

void Mostrar(void *param){
	while(1){

		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		float pitch = CalcularPitch(x_prom, y_prom, z_prom);
		float roll = CalcularRoll(x_prom, y_prom, z_prom);

		printf("\nX: %.2f g\n", x_prom);
		printf("Y: %.2f g\n", y_prom);
		printf("Z: %.2f g\n", z_prom);
		printf("\nPitch: %.2f\n", pitch);
		printf("Roll: %.2f\n", roll);
	}
}

float CalcularPitch(float x, float y, float z){
	float pitch = 0;
	pitch = atan(y/sqrt((pow(x,2)+pow(z,2))))*180/3.1415;
	return pitch;
}

float CalcularRoll(float x, float y, float z){
	float roll = 0;
	roll = atan(x/sqrt((pow(y,2)+pow(z,2))))*180/3.1415;
	return roll;
}

// void calibracion(){

// }

/*==================[external functions definition]==========================*/

void app_main(void)
{
	
	ADXL335Init();
	
	timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = MEDICION_PERIODO_US,
        .func_p = FuncTimer,
        .param_p = NULL
    };
	// TimerInit(&timer_medicion);

	// timer_config_t timer_muestreo = {
    //     .timer = TIMER_B,
    //     .period = (MUESTREO_PERIODO_US-MEDICION_PERIODO_US*n), //Deja un total de 1 segundo teniendo en cuenta el tiempo de medición
    //     .func_p = FuncTimer,
    //     .param_p = NULL
    // };
	// TimerInit(&timer_muestreo);

	xTaskCreate(Medir, "Medir aceleraciones", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(Mostrar, "Mostrar pitch y roll", 2048, NULL, 5, &mostrar_task_handle);

	// TimerStart(TIMER_A);
	// TimerStart(TIMER_B);

	
}
/*==================[end of file]============================================*/