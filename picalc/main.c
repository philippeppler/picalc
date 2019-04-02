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

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"


extern void vApplicationIdleHook( void );
void vDisplay(void *pvParameters);

TaskHandle_t DisplayTask;

void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
   // resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
	xTaskCreate( vDisplay, (const char *) "DisplayTask", configMINIMAL_STACK_SIZE, NULL, 1, &DisplayTask);

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI Calculator");
	vDisplayWriteStringAtPos(1,0,"Philipp Eppler");
	vDisplayWriteStringAtPos(2,0,"Pi: 3.1415xxx");
	vDisplayWriteStringAtPos(3,0,"Zeit: xxxxxxms");
	vTaskStartScheduler();
	return 0;
}

void vDisplay(void *pvParameters) {
	(void) pvParameters;
	PORTF.DIRSET = PIN0_bm; /*LED1*/
	PORTF.OUT = 0x01;
	for(;;) {
// 		uint32_t stack = get_mem_unused();
// 		uint32_t heap = xPortGetFreeHeapSize();
// 		uint32_t taskStack = uxTaskGetStackHighWaterMark(ledTask);
// 		vDisplayClear();
// 		vDisplayWriteStringAtPos(0,0,"Stack: %d", stack);
// 		vDisplayWriteStringAtPos(1,0,"Heap: %d", heap);
// 		vDisplayWriteStringAtPos(2,0,"TaskStack: %d", taskStack);
// 		vDisplayWriteStringAtPos(3,0,"FreeSpace: %d", stack+heap);
		PORTF.OUTTGL = 0x01;				
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}
