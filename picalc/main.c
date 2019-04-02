/*
 * picalc.c
 *
 * Created: 02.04.2019 18:57:07
 * Author : philippeppler
 */ 

//#include <avr/io.h>
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


extern void vApplicationIdleHook( void );
void vGUI(void *pvParameters);

TaskHandle_t GUITask;

void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	vInitClock();
	vInitDisplay();
	
	xTaskCreate( vGUI, (const char *) "GUITask", configMINIMAL_STACK_SIZE, NULL, 2, &GUITask);

	vTaskStartScheduler();
	return 0;
}

void vGUI(void *pvParameters) {
	(void) pvParameters;
	PORTF.DIRSET = PIN0_bm; /*LED1*/
	PORTF.OUT = 0x01;
	for(;;) {
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0,"PI Calculator");
		vDisplayWriteStringAtPos(1,0,"Philipp Eppler");
		vDisplayWriteStringAtPos(2,0,"Pi: 3.1415xxx");
		vDisplayWriteStringAtPos(3,0,"Zeit: xxxxxxms");
		
		PORTF.OUTTGL = 0x01;				
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}
