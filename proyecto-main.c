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
#include "USART.h"
#include "LCD4b.h"

#define _XTAL_FREQ 1000000


// VARIABLES GLOBALES
uint8_t Fosc = 1;                                                               // SELECCIONAR FRECUENCIA DE OSCILADOR
int PS_val = 16;                                                            // VALOR DEL PRESCALER DEL TMR0
uint8_t cont = 0;                                                              // VALOR DEL CONTADOR DEL TMR0
uint8_t milis = 0;
uint8_t pulse = 0;
uint8_t spst = 0;
uint8_t HS_flag = 0;
uint8_t sensor_flag = 0;
uint8_t spst_flag = 0;
char HS_flag_str[10];
char spst_str[10];
uint8_t frerr = 0;
uint8_t overr = 0;

// PROTOTIPO DE FUNCIONES
void setup(void);
void stepSet(uint8_t push, uint8_t set_pulse);

//INTERRUPCIONES
void __interrupt() isr(void){
    
    // INTERRUPCION DEL TMR0 PARA GENERAR PULSOS DE 20mS
    if (T0IF){
        milis++;
        if (milis == 4){
            pulse = !pulse;
            milis = 0;            
        }
        tmr0Reset();
    }
    
    // INTERRUPCION PARA RECEPCION DE VALORES DE LOS SENSORES
    if (RCIF){
        frerr = RCSTAbits.FERR;
        overr = RCSTAbits.OERR;
        sensor_flag = RCREG;
        sprintf(spst_str, "%d", sensor_flag);
        RCREG = 0;
        if (sensor_flag == 65){
            frerr = RCSTAbits.FERR;
            overr = RCSTAbits.OERR;
            PORTAbits.RA0 = frerr;
            PORTAbits.RA1 = overr;
            HS_flag = RCREG;
            sprintf(HS_flag_str, "%d", HS_flag);
            RCREG = 0;
        }
//        frerr = RCSTAbits.FERR;
//        overr = RCSTAbits.OERR;
//        sensor_flag = RCREG;
//        RCREG = 0;
//        if (sensor_flag == 66){
//            frerr = RCSTAbits.FERR;
//            overr = RCSTAbits.OERR;
//            spst_flag = RCREG;
//            sprintf(spst_str, "%d", spst_flag);
//            RCREG = 0;
//        }
    }
}

void main(void) {
    setup();
    initOscFreq(Fosc);
    initTmr0(PS_val);
    usartInitTransmit();
    Lcd_Init();
    
    while(1){
//        stepSet(spst, pulse);
        Lcd_Set_Cursor(0,9);
        Lcd_Write_String(spst_str);
        if (HS_flag == 48){
            Lcd_Set_Cursor(0,2);
            Lcd_Write_String(HS_flag_str);
        }        
        else if (HS_flag == 49){
            Lcd_Set_Cursor(0,2);
            Lcd_Write_String(HS_flag_str);
        }               
    }
    return;
}

void setup(void){
    //IO CONFIG
    ANSEL = 0;
    ANSELH = 0;
    
    TRISA = 0;
    TRISB = 0;
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void stepSet(uint8_t push, uint8_t set_pulse){
    if (push == 1){
        PORTCbits.RC0 = set_pulse;
    }
    else {
        PORTCbits.RC0 = 0;
    }
}