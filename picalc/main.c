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

#include "mem_check.h"
#include "queue.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"
#include "ButtonHandler.h"


extern void vApplicationIdleHook( void );
void vUI(void *pvParameters);
void vButton(void *pvParameters);

xQueueHandle ButtonQueue;



void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	//resetReason_t reason = getResetReason();
	
	vInitClock();
	vInitDisplay();
	
	xTaskCreate( vButton, (const char *) "Button", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate( vUI, (const char *) "GUITask", configMINIMAL_STACK_SIZE+100, NULL, 1, NULL);
	
	ButtonQueue=xQueueCreate(20, sizeof(char));
	
	PORTF.DIRSET = PIN0_bm;						//LED1
	
	vTaskStartScheduler();
	return 0;
}

void vUI(void *pvParameters) {
	char but = '0';
	while (1) {
		if (uxQueueMessagesWaiting(ButtonQueue) > 0) {
			xQueueReceive(ButtonQueue, &but, portMAX_DELAY);
		} else {
			but = '0';
		}
		
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0,"Queues");
		vDisplayWriteStringAtPos(1,0,"Anzahl: %d", uxQueueMessagesWaiting(ButtonQueue));
		vDisplayWriteStringAtPos(2,0,"Pressed: %c", but);
		vTaskDelay(3000/portTICK_RATE_MS);							// Delay 3s
	}
}

void vButton(void *pvParameters) {
	initButtons();
	char x;
	
	while (1) {
		updateButtons();
		if (getButtonPress(BUTTON1) == SHORT_PRESSED) {				// Wenn Button1 gedrückt
			x = '1';	
		} else if (getButtonPress(BUTTON2) == SHORT_PRESSED) {		// Wenn Button2 gedrückt
			x = '2';
		} else if (getButtonPress(BUTTON3) == SHORT_PRESSED) {		// Wenn Button3 gedrückt
			x = '3';
		} else {
			x = '0';
		}
		if (x != '0') {
			xQueueSendToBack(ButtonQueue, &x, portMAX_DELAY);
		}

		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);	// Delay 10ms
	}
}