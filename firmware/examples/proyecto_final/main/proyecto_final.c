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
 * | 22/10/2025 | Creacion del documento	                     |
 *
 * @author Juana Nasi (juananasi3009@gmail.com)
 * @author Josefina Nicola (josefina.nicola@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include "ADXL335.h"
#include <stdio.h>
#include "analog_io_mcu.h"
#include "freertos/FreeRTOS.h"
#include <math.h>
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/


/*==================[internal data definition]===============================*/



/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/

float CalcularAnguloX(float x_0, float x){
	float angulo_x = 0;
	angulo_x = atan(x/x_0) * (180.0 / 3.1416); //calculo del angulo de radianes a grados
	return angulo_x;
}

float CalcularAnguloY(float y_0, float y){
	float angulo_y = 0;
	angulo_y = atan(y/y_0) * (180.0 / 3.1416); //calculo del angulo de radianes a grados
	return angulo_y;
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	ADXL335Init();
	while(1){
		int x = ReadXValueInt();
		//printf("\nX: %.2f g\n", x);
		printf("\nX: %d\n", x);
		int y = ReadYValueInt();
		//printf("Y: %.2f g\n", y);
		printf("Y: %d\n", y);
		int z = ReadZValueInt();
		//printf("Z: %.2f g\n", z);
		printf("Z: %d\n", z);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
/*==================[end of file]============================================*/