/*
 * File:   usart-main.c
 * Author: Luis Genaro Alvarez Sulecio
 * Description: 
 * Created on September 13, 2022, 10:21 AM
 */
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include "osc_config.h"
#include "tmr0_config.h"

#define _XTAL_FREQ 1000000


// VARIABLES GLOBALES
uint8_t Fosc = 1;                                                               // SELECCIONAR FRECUENCIA DE OSCILADOR
int PS_val = 16;                                                            // VALOR DEL PRESCALER DEL TMR0
uint8_t cont = 0;                                                              // VALOR DEL CONTADOR DEL TMR0
uint8_t milis = 0;
uint8_t pulse = 0;
uint8_t spst = 0;

// PROTOTIPO DE FUNCIONES
void setup(void);
void stepSet(uint8_t push, uint8_t set_pulse);

//INTERRUPCIONES
void __interrupt() isr(void){
    if (T0IF){
        milis++;
        if (milis == 4){
            pulse = !pulse;
            milis = 0;            
        }
        tmr0Reset();
    }
    if (RBIF){
        if (!PORTBbits.RB0){
            spst = 1;
        }
        else {
            spst = 0;
        }
        INTCONbits.RBIF = 0;
    }
}

void main(void) {
    setup();
    initOscFreq(Fosc);
    initTmr0(PS_val);
    while(1){
        stepSet(spst, pulse);
    }
    return;
}

void setup(void){
    //IO CONFIG
    ANSEL = 0;
    ANSELH = 0;
    
    // CONFIGURACION DE PUERTO B PARA PUSHBUTTON
    TRISB = 0b00000001;
    OPTION_REGbits.nRBPU = 0;                                                   // HABILITAR WEAK PULLUP EN PUERTO B
    WPUB = 0b00000001;                                                          // HABILITAR RESISTENCIA EN RB0 
    IOCBbits.IOCB0 = 1;                                                         // HABILITAR INTERRUPCION EN CAMBIO PARA RB0
    
    TRISC = 0;
    TRISD = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;
}

void stepSet(uint8_t push, uint8_t set_pulse){
    if (push == 1){
        PORTCbits.RC0 = set_pulse;
    }
    else {
        PORTCbits.RC0 = 0;
    }
}