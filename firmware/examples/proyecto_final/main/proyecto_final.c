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

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/


/*==================[internal data definition]===============================*/



/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/

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
	int n = 4;
	
	ADXL335Init();
	
	while(1){
		float x_prom = 0;
		float y_prom = 0;
		float z_prom = 0;
		
		for (int i = 0; i < n; i++){
			x_prom += ReadXValue()/n;
			y_prom += ReadYValue()/n;
			z_prom += ReadZValue()/n;

			vTaskDelay(10 / portTICK_PERIOD_MS);	// hacer función delay diez ms
		}

		float pitch = CalcularPitch(x_prom, y_prom, z_prom);
		float roll = CalcularRoll(x_prom, y_prom, z_prom);
		
		printf("\nX: %.2f g\n", x_prom);
		printf("Y: %.2f g\n", y_prom);
		printf("Z: %.2f g\n", z_prom);
		printf("\nPitch: %.2f\n", pitch);
		printf("Roll: %.2f\n", roll);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
/*==================[end of file]============================================*/