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

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"
#include "ButtonHandler.h"

#include "avr_f64.h"												//Library Include


// EventGroup
#define STARTCALC	1<<0
#define RESETCALC	1<<2
#define FINISHCALC	1<<3
#define TICK		1<<4
EventGroupHandle_t egPiStates;

extern void vApplicationIdleHook( void );
void vGUI(void *pvParameters);
void vButton(void *pvParameters);
void vCalc(void *pvParameters);

TaskHandle_t GUITask;

float64_t dPi4;
long i;
long Timems;


void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	vInitClock();
	vInitDisplay();
	egPiStates = xEventGroupCreate();
	
	xTaskCreate( vButton, (const char *) "Button", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate( vGUI, (const char *) "GUITask", configMINIMAL_STACK_SIZE, NULL, 2, &GUITask);
	xTaskCreate( vCalc, (const char *) "Calc", configMINIMAL_STACK_SIZE+300, NULL, 1, NULL);

	PORTF.DIRSET = PIN0_bm;						//LED1
	
	vTaskStartScheduler();
	return 0;
}

void vGUI(void *pvParameters) {
	(void) pvParameters;
	char sPi[17] = "";			
	char Iter[15] = "";
	char sTime[5] = "";
	for(;;) {
		
		xEventGroupClearBits(egPiStates, FINISHCALC);
		if (dPi4 != 1) {
			//long dPi4Temp = dPi4 * 4;
			char* stempResult = f_to_string(dPi4, 16, 16);
			sprintf(sPi, "%s", stempResult)	;
		}
		else {
			sprintf(sPi, "press start");
		}
		sprintf(Iter, "%ld", i);
		sprintf(sTime, "%ld", Timems);
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0,"PI Calculator");
		vDisplayWriteStringAtPos(1,0,"%s", Iter);
		vDisplayWriteStringAtPos(2,0,"Pi: %s", sPi);
		vDisplayWriteStringAtPos(3,0,"Zeit: %s ms",sTime);
		xEventGroupSetBits(egPiStates, FINISHCALC);
		
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}

void vButton(void *pvParameters) {
	initButtons();
	
	while (1) {
		updateButtons();
		if (getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(egPiStates, STARTCALC);
			TCD0.CTRLA = TC_CLKSEL_DIV1_gc ;						// Timer starten
		}

		if (getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupClearBits(egPiStates, STARTCALC);
			TCD0.CTRLA = TC_CLKSEL_OFF_gc ;							// Timer stoppen
		}
		
		if (getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(egPiStates, RESETCALC);
		}		

		if (getButtonPress(BUTTON4) == SHORT_PRESSED) {

		}

		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
	}
}

void vCalc(void *pvParameters) {
	dPi4 = 1;
	uint16_t calcstate = 0x0000;
	i = 0;
	
	
	TCD0.CTRLA = TC_CLKSEL_OFF_gc ;
	TCD0.CTRLB = 0x00;
	TCD0.INTCTRLA = 0x03;
	TCD0.PER = 32000-1;							// Zeit mit KO kontrollieren, allenfalls hier korrigieren
	
	for(;;) {
		
		calcstate = xEventGroupGetBits(egPiStates);
		
		if (calcstate & FINISHCALC) {
			if (calcstate & STARTCALC) {
				dPi4 = dPi4 - (1.0/(3+4*i)) + (1.0/(5+4*i));
				i++;
				if (dPi4 < 0.7854 ) {
					TCD0.CTRLA = TC_CLKSEL_OFF_gc ;
				}
			}
		}

		if (calcstate & RESETCALC) {
			dPi4 = 1;
			i = 0;
			Timems = 0;
			xEventGroupClearBits(egPiStates, RESETCALC);
			TCD0.CTRLA = TC_CLKSEL_DIV1_gc ;						// Timer starten
		}
	}
}
	

ISR(TCD0_OVF_vect)
{
	Timems++;
	PORTF.OUTTGL = PIN0_bm;			//LED1
}