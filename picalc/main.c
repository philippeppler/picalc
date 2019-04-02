/*
 * picalc.c
 *
 * Created: 02.04.2019 18:57:07
 * Author : philippeppler
 */ 

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"
//#include "math.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"
#include "ButtonHandler.h"

#define ITERATIONS 100000

extern void vApplicationIdleHook( void );
void vGUI(void *pvParameters);
void vButton(void *pvParameters);
void vCalc(void *pvParameters);

TaskHandle_t GUITask;

double dPi4; 
long i;


void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	vInitClock();
	vInitDisplay();
	
	xTaskCreate( vButton, (const char *) "Button", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate( vGUI, (const char *) "GUITask", configMINIMAL_STACK_SIZE, NULL, 2, &GUITask);
	xTaskCreate( vCalc, (const char *) "Calc", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	vTaskStartScheduler();
	return 0;
}

void vGUI(void *pvParameters) {
	char Pi[10] = "";			
	char Iter[15] = "";
	for(;;) {

		sprintf(Pi, "%f", 4*dPi4);
		sprintf(Iter, "%ld", i);
		
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0,"PI Calculator");
		vDisplayWriteStringAtPos(1,0,"%s", Iter);
		vDisplayWriteStringAtPos(2,0,"Pi: %s", Pi);
		vDisplayWriteStringAtPos(3,0,"Zeit: xxxxxxms");
		

		vTaskDelay(500 / portTICK_RATE_MS);
	}
}

void vButton(void *pvParameters) {
	initButtons();
	
	while (1) {
		updateButtons();
		if (getButtonPress(BUTTON1) == SHORT_PRESSED) {

		}

		if (getButtonPress(BUTTON2) == SHORT_PRESSED) {

		}
		
		if (getButtonPress(BUTTON3) == SHORT_PRESSED) {

		}		

		if (getButtonPress(BUTTON4) == SHORT_PRESSED) {

		}

		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
	}
}

void vCalc(void *pvParameters) {
	dPi4 = 1;
	
	for(i = 0;i<ITERATIONS;i++) {
		dPi4 = dPi4 - (1.0/(3+4*i)) + (1.0/(5+4*i));
		vTaskDelay(1/portTICK_RATE_MS);
	}
	
}
	
	