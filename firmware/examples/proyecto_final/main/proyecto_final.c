/*! @mainpage Proyecto Final
 *
 * @section Monitor de postura
 *
 * El programa busca notificarle al usuario, mediante la vibración de un motor, de su mala postura.
 * Funciona mediante dos tareas. La tarea principal busca tomar las mediciones de aceleración, obteniéndolas
 * de un acelerómetro. Calcula el promedio de las N primeras medidas tomadas para reducir el ruido eléctrico.
 * Estas medidadas son enviadas a la segunda tarea que realizará los cálculos de los ánculos pitch y roll.
 * Se comparan estos valores con los valores de referencia, que almacenan el valor del ángulo en una postura
 * saludable. Si el desvío es significativo durante 20 segundos, se envía una vibración que dura 3 segundos.
 * 
 * Los valores de referencia son tomados cuando comienza a correr el programa. Los tiempos de muestreo son
 * manejados con Timers.
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
 * | 05/11/2025 | Implementación correcta de Timers	             |
 * | 12/11/2025 | Agregado de calibración para valores de		 |
 * |			| referencia, lógica de vibración y sensibilidad |
 * |			| de los ángulos	             				 |
 * | 12/11/2025 | Finalización del proyecto integrador			 |
 * | 14/11/2025 | Se agrega documentación del proyecto			 |
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
#include "l293.h"
#include "pwm_mcu.h"

/*==================[macros and definitions]=================================*/
/** @def MUESTREO_PERIODO_US
 * @brief Valor del período de muestreo en microsegundos
 */
#define MUESTREO_PERIODO_US 1000000

/** @def N_MUESTRAS
 * @brief Valor de la cantidad de muestras a tomar para la reducción del ruido eléctrico
 */
#define N_MUESTRAS 4 

/*==================[internal data declaration]==============================*/

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

float CalcularPitch(float x, float y, float z);
float CalcularRoll(float x, float y, float z);
void Vibrar(float pitch, float roll);

/*==================[internal data definition]===============================*/

float x_prom;
float y_prom; 
float z_prom;

int cont = 0;

float pitch_ref = 0;
float roll_ref = 0;

/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/

/** @fn void FuncTimer(void* param)
 * @brief Envía notificaciones a la tarea asociada a Medir
 * @return
 */
void FuncTimer(void *param){
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

/** @fn void Medir(void *param)
 * @brief Obtiene los valores de aceleración del acelerómetro y cálcula un promedio de N_MUESTRAS de las medidas para reducir el ruido eléctrico
 * @return
 */
void Medir(void *param){
	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		x_prom = 0;
		y_prom = 0;
		z_prom = 0;
			
		for (int i = 0; i < N_MUESTRAS; i++){
			x_prom += ReadXValue() / N_MUESTRAS;
			y_prom += ReadYValue() / N_MUESTRAS;
			z_prom += ReadZValue() / N_MUESTRAS;
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		xTaskNotifyGive(mostrar_task_handle);
	}
}

/** @fn void Mostrar(void *param)
 * @brief Se encarga de calcular pitch y roll, los muestra por consola y se los pasa a la función Vibrar()
 * @return
 */
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

		Vibrar(pitch, roll);
	}
}

/** @fn void Vibrar(float pitch, float roll)
 * @brief Calcula la desviación entre los valores de pitch y roll actuales y los valores estándar. Envía una vibración si la desviación es significativa durante más de 20 segundos
 * @param pitch Flotante que almacena el ángulo pitch
 * @param roll Flotante que almacena el ángulo roll
 * @return
 */
void Vibrar(float pitch, float roll){
	
	float desviacion_pitch = fabs(pitch - pitch_ref);
	float desviacion_roll = fabs(roll - roll_ref);
	
	if (desviacion_pitch > 10 || desviacion_roll > 7){
		cont++;
	}

	if (cont > 20){
		L293SetSpeed(MOTOR_2, 90);
		vTaskDelay(3000 / portTICK_PERIOD_MS); // Durante este tiempo no mide, tiempo al usuario para enderezarse
		L293SetSpeed(MOTOR_2, 0);
		cont = 0;
	}
}
/** @fn float CalcularPitch(float x, float y, float z)
 * @brief Calcula el ángulo pitch
 * @param x Aceleración en el eje x
 * @param y Aceleración en el eje y
 * @param z Aceleración en el eje z
 * @return Flotante que almacena el valor de pitch
 */
float CalcularPitch(float x, float y, float z){
	float pitch = 0;
	pitch = atan(y/sqrt((pow(x,2)+pow(z,2))))*180/3.1415;
	return pitch;
}

/** @fn float CalcularRoll(float x, float y, float z)
 * @brief Calcula el ángulo roll
 * @param x Aceleración en el eje x
 * @param y Aceleración en el eje y
 * @param z Aceleración en el eje z
 * @return Flotante que almacena el valor de roll
 */
float CalcularRoll(float x, float y, float z){
	float roll = 0;
	roll = atan(x/sqrt((pow(y,2)+pow(z,2))))*180/3.1415;
	return roll;
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	ADXL335Init();
	L293Init();

	x_prom = 0;
	y_prom = 0;
	z_prom = 0;
		
	for (int i = 0; i < N_MUESTRAS; i++){
		x_prom += ReadXValue() / N_MUESTRAS;
		y_prom += ReadYValue() / N_MUESTRAS;
		z_prom += ReadZValue() / N_MUESTRAS;
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	pitch_ref = CalcularPitch(x_prom, y_prom, z_prom);
	roll_ref = CalcularRoll(x_prom, y_prom, z_prom);
	
	timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = MUESTREO_PERIODO_US,
        .func_p = FuncTimer,
        .param_p = NULL
    };
	TimerInit(&timer_medicion);

	xTaskCreate(Medir, "Medir aceleraciones", 4096, NULL, 5, &medir_task_handle);
	xTaskCreate(Mostrar, "Mostrar pitch y roll", 4096, NULL, 5, &mostrar_task_handle);

	TimerStart(TIMER_A);
}
/*==================[end of file]============================================*/
