/*
 * File:   tmr0_config.c
 * Author: Ing. Luis Genaro Alvarez Sulecio
 * Revision history: ULTIMA ACTUALIZACION REALIZADA EL 12/08/2022 | SE CAMBIO EL TIPO DE VARIABLE DEL PRESCALER A UNA TIPO "INT" PARA QUE SE LE PUEDA DAR EL VALOR CORRECTO. ADICIONALMENTE
 * SE HABILITO LA INTERRUPCION DEL TMR0 DENTRO DE LA FUNCION initTmr0.
 * Created on July 22, 2022, 12:49 PM
 */

#include "tmr0_config.h"
#include <xc.h>
#include <stdio.h>
#include <stdint.h>

void initTmr0 (int prescaler) {
    switch(prescaler){
        case 0:
            break;
        case 2:
            OPTION_REGbits.PS = 0b000;     // COLOCAR PRESCALER EN 1:2
            break;
        case 4:
            OPTION_REGbits.PS = 0b001;     // COLOCAR PRESCALER EN 1:4
            break;
        case 8:
            OPTION_REGbits.PS = 0b010;     // COLOCAR PRESCALER EN 1:8
            break;
        case 16:
            OPTION_REGbits.PS = 0b011;     // COLOCAR PRESCALER EN 1:16
            break;
        case 32:
            OPTION_REGbits.PS = 0b100;     // COLOCAR PRESCALER EN 1:32
            break;
        case 64:
            OPTION_REGbits.PS = 0b101;     // COLOCAR PRESCALER EN 1:64
            break;
        case 128:
            OPTION_REGbits.PS = 0b110;     // COLOCAR PRESCALER EN 1:128
            break;
        case 256:
            OPTION_REGbits.PS = 0b111;     // COLOCAR PRESCALER EN 1:256
            break;
        default:
            break;
    }
    
    OPTION_REGbits.T0CS = 0;               // UTILIZAR CICLO INTERNO
    OPTION_REGbits.PSA = 0;                // CAMBIAR PRESCALER A TMR0
    INTCONbits.T0IE = 1;                   // HABILITAR INTERRUPCIONES DEL TMR0
    INTCONbits.T0IF = 0;                   // LIMPIAR BANDERA DE INTERRUPCION EN TMR0
    TMR0 = tmr0_val;                       // VALOR DE TMR0
}

void tmr0Reset(void) {
    INTCONbits.T0IF = 0;                   // LIMPIAR BANDERA DE INTERRUPCION EN TMR0
    TMR0 = tmr0_val;                       // VALOR DE TMR0
}