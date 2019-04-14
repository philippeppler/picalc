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

// EventGroup
#define STARTCALC	1<<0
#define RESETCALC	1<<2
#define FINISHCALC	1<<3
EventGroupHandle_t egPiStates;

extern void vApplicationIdleHook( void );
void vUI(void *pvParameters);
void vButton(void *pvParameters);
void vCalc(void *pvParameters);

TaskHandle_t GUITask;

double dPi4;			// Pi/4 für Berechnung mit Leibniz-Reihe
long i;					// Anzahl Iterationen
long Timems;			// 1ms-Zähler


void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	//resetReason_t reason = getResetReason();
	
	vInitClock();
	vInitDisplay();
	egPiStates = xEventGroupCreate();
	
	xTaskCreate( vButton, (const char *) "Button", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate( vUI, (const char *) "GUITask", configMINIMAL_STACK_SIZE+100, NULL, 2, NULL);
	xTaskCreate( vCalc, (const char *) "Calc", configMINIMAL_STACK_SIZE+100, NULL, 1, NULL);

	PORTF.DIRSET = PIN0_bm;						//LED1
	
	vTaskStartScheduler();
	return 0;
}

void vUI(void *pvParameters) {
	char Pi[15] = "";			
	char Iter[15] = "";
	char sTime[5] = "";
	TickType_t xLastWakeTime;
	
	while (1) {
		
		xEventGroupClearBits(egPiStates, FINISHCALC);			// FINISHCALC = 0
		if (dPi4 != 1) {										// wenn nicht Initialwert
			sprintf(Pi, "%f", 4*dPi4);								// Pi als String ausgeben
		}
		else {
			sprintf(Pi, "press start");								// Sonst "press start" ausgeben
		}
		sprintf(Iter, "%ld", i);
		sprintf(sTime, "%ld", Timems);
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0,"PI Calculator");
		vDisplayWriteStringAtPos(1,0,"%s", Iter);
		vDisplayWriteStringAtPos(2,0,"Pi: %s", Pi);
		vDisplayWriteStringAtPos(3,0,"Zeit: %s ms",sTime);
		xEventGroupSetBits(egPiStates, FINISHCALC);				// FINISHCALC = 1
		
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_RATE_MS); // Pausenzeit genau 500ms
	}
}

void vButton(void *pvParameters) {
	initButtons();
	
	while (1) {
		updateButtons();
		if (getButtonPress(BUTTON1) == SHORT_PRESSED) {				// Wenn Button1 gedrückt
			xEventGroupSetBits(egPiStates, STARTCALC);					// STARTCALC = 1
			TCD0.CTRLA = TC_CLKSEL_DIV1_gc ;							// Timer starten
		}

		if (getButtonPress(BUTTON2) == SHORT_PRESSED) {				// Wenn Button2 gedrückt
			xEventGroupClearBits(egPiStates, STARTCALC);				// STARTCALC = 0
			TCD0.CTRLA = TC_CLKSEL_OFF_gc ;								// Timer stoppen
		}
		
		if (getButtonPress(BUTTON3) == SHORT_PRESSED) {				// Wenn Button3 gedrückt
			xEventGroupSetBits(egPiStates, RESETCALC);					// RESETCALC = 1
		}		


		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);	// Delay 10ms
	}
}

void vCalc(void *pvParameters) {
	dPi4 = 1;
	uint16_t calcstate = 0x0000;
	i = 0;
	
	
	TCD0.CTRLA = TC_CLKSEL_OFF_gc ;				// 1ms Timer Initialisieren, aber nicht starten
	TCD0.CTRLB = 0x00;
	TCD0.INTCTRLA = 0x03;
	TCD0.PER = 32000-1;								// Zeit mit KO kontrollieren, allenfalls hier korrigieren
	
	while (1) {
		
		calcstate = xEventGroupGetBits(egPiStates);				// EventBits aufrufen
		
		if (calcstate & FINISHCALC) {							// Wenn FINISHCALC = 1
			if (calcstate & STARTCALC) {							// Wenn STARTCALC = 1
				dPi4 = dPi4 - (1.0/(3+4*i)) + (1.0/(5+4*i));		// Pi mit Leibniz-Reihe berechnen
				i++;
				if (dPi4 < 0.7854 ) {								// Wenn Pi auf 5 Kommastellen genau
					TCD0.CTRLA = TC_CLKSEL_OFF_gc ;						// 1ms Timer stoppen
				}
			}
		}
		if (calcstate & RESETCALC) {							// Wenn RESETCALC = 1
				dPi4 = 1;											// auf Initialwerte zurücksetzen
				i = 0;
				Timems = 0;
				xEventGroupClearBits(egPiStates, RESETCALC);
		}
	}
}
	

ISR(TCD0_OVF_vect)
{
	Timems++;													// 1ms-Zähler inkrementieren
	PORTF.OUTTGL = PIN0_bm;										//LED1 toggeln
}