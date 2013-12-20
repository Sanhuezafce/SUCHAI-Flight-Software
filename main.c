/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2013, Carlos Gonzalez Cortes, carlgonz@ug.uchile.cl
 *      Copyright 2013, Tomas Opazo Toro, tomas.opazo.t@gmail.com 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__XC16__)
    #include <xc.h>
#else
    #include <p24FJ256GA110.h>
#endif

#include <stdio.h>
#include <PPS.h>

/* Drivers includes */

/* System includes */
#include "SUCHAI_config.h"

/* RTOS Includes */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "list.h"

/* Task includes */
#include "taskTest.h"
#include "taskDispatcher.h"
#include "taskExecuter.h"
#include "taskHouskeeping.h"

/* Config Words */
// CONFIG3
#pragma config WPFP = WPFP511           // Write Protection Flash Page Segment Boundary (Highest Page (same as page 170))
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable bit (Segmented code protection disabled)
#pragma config WPCFG = WPCFGDIS         // Configuration Word Code Page Protection Select bit (Last page(at the top of program memory) and Flash configuration words are not protected)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select bit (Write Protect from WPFP to the last page of memory)

// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT oscillator mode selected)
#pragma config I2C2SEL = PRI            // I2C2 Pin Select bit (Use Default SCL2/SDA2 pins for I2C2)
#pragma config IOL1WAY = OFF            // IOLOCK One-Way Set Enable bit (Unlimited Writes To RP Registers)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSCO functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Both Clock Switching and Fail-safe Clock Monitor are disabled)
#pragma config FNOSC = PRIPLL           // Oscillator Select (Primary oscillator (XT, HS, EC) with PLL module (XTPLL,HSPLL, ECPLL))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-speed start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Standard Watchdog Timer is enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = ON              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator functions are shared with PGEC1/PGED1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

/* Global variables */
xSemaphoreHandle dataRepositorySem, consolePrintfSem;
xQueueHandle dispatcherQueue, executerCmdQueue, executerStatQueue;

static void on_reset(void);

int main(void)
{
    /* Initializing shared Queues */
    dispatcherQueue = xQueueCreate(25,sizeof(DispCmd));
    executerCmdQueue = xQueueCreate(1,sizeof(ExeCmd));
    executerStatQueue = xQueueCreate(1,sizeof(int));

    /* Initializing shared Semaphore */
    dataRepositorySem = xSemaphoreCreateMutex();
    consolePrintfSem = xSemaphoreCreateMutex();

    /* Crating all tasks */
    int node = 1;
    xTaskCreate(taskTestCSP, (signed char *)"CSP", 2*configMINIMAL_STACK_SIZE, (void *)(&node), 3, NULL);
    xTaskCreate(taskDispatcher, (signed char *)"dispatcher", 2*configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(taskExecuter, (signed char *)"executer", 5*configMINIMAL_STACK_SIZE, NULL, 4, NULL);
//    xTaskCreate(taskHouskeeping, (signed char *)"housekeeping", 2*configMINIMAL_STACK_SIZE, NULL, 2, NULL);


//    xTaskCreate(taskTest, (signed char*)"taskTest", configMINIMAL_STACK_SIZE, (void *)"T1 Running...", 1, NULL);
//    xTaskCreate(taskTest, (signed char*)"taskTest", configMINIMAL_STACK_SIZE, (void *)"T2 Running...", 2, NULL);

    /* Configure Peripherals */

    /* On reset */
    on_reset();


    /* Start the scheduler. Should never return */
    printf(">>Starting FreeRTOS [->]\r\n");
    vTaskStartScheduler();

    while(1)
    {
        printf("\n>>FreeRTOS [FAIL]\n");
    }
    
    return 0;
}

/**
 * Task idle handle function. Performs operations inside the idle task
 * configUSE_IDLE_HOOK must be set to 1
 */
void vApplicationIdleHook(void)
{
    ClrWdt();
}

/**
 * Stack overflow handle function.
 * configCHECK_FOR_STACK_OVERFLOW must be set to 1 or 2
 * 
 * @param pxTask Task handle
 * @param pcTaskName Task name
 */
void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    printf(">> Stak overflow! - TaskName: %s\n", (char *)pcTaskName);
    
    /* Stack overflow handle */
    while(1);
}

void configure_ports(void)
{
    /* CUBESAT KIT MB CONFIGURATION */
    //-OE_USB -INT //6	RPI38/CN45/RC1	=> pin es RC1
    _TRISC1=0; 			//pin 0-output 1-input.
    //-OE_MHX //98	CN60/PMD2/RE2	=> pin es RE2
    _TRISE2=0; 			//pin 0-output 1-input.
    //-ON_MHX //99	CN61/PMD3/RE3	=> pin es RE3
    _TRISE3=0; 			//pin 0-output 1-input.
    //-ON_SD //100	CN62/PMD4/RE4	=> pin es RE4
    _TRISE4=0; 			//pin 0-output 1-input.

    _LATE2 = 1; 		/* -OE_MHX OFF */
    _LATE3 = 0; 		/* -ON_MHX ON */
    _LATC1 = 0; 		/* -OE_USB ON */

    //Conifg para Consola:
    // H1.17 - U1RX - RP10 - IO.7 - UART 1 PARA CONSOLA SERIAL
    iPPSInput(IN_FN_PPS_U1RX,IN_PIN_PPS_RP10);
    // H1.18 - U1TX - RP17 - IO.6 - UART 1 PARA CONSOLA SERIAL
    iPPSOutput(OUT_PIN_PPS_RP17,OUT_FN_PPS_U1TX);
}

/**
 * Performs initialization actions
 */
void on_reset(void)
{
    configure_ports(); //Configure hardware
    repo_onResetCmdRepo(); //Command repository initialization
    dat_onResetCubesatVar(); //Update status repository

    /* UART1 - CONSOLE - 19200, 8, N, 1 */
    ConfigRS232(51, RS2_M_UART1);
    EnableIntU1RX;
    SetPriorityIntU1RX(5);
}


#define STDIN   0
#define STDOUT  1
#define STDERR  2
#define LF   '\n'
#define CR   '\r'

void    mon_putc(char ch);

int __attribute__((__weak__, __section__(".libc")))
write(int handle, void * buffer, unsigned int len)
{
    int i = 0;
    switch (handle)
    {
        case STDOUT:
        case STDERR:
            while (i < len)
                mon_putc(((char*)buffer)[i++]);
            break;
    }
    return (len);  // number of characters written
}

#define STDOUT_NO_CR_WITH_LF
void mon_putc(char ch)
{
    while(U1STAbits.UTXBF);  /* wait if the buffer is full */
#ifndef STDOUT_NO_CR_WITH_LF
    if (LF == ch)
        putcUART1(CR);
#endif
    putcUART1(ch);
}