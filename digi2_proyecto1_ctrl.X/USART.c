/*
 * File:   USART.c
 * Author: luisg
 *
 * Created on September 13, 2022, 11:06 AM
 */

#ifndef _XTAL_FREQ
#define	_XTAL_FREQ 1000000
#endif

#include "USART.h"
#include <xc.h>
#include <stdio.h>
#include <stdint.h>

// PARA INICIALIZAR LA COMUNICACION USART EN MODO DE TRANSMISOR
void usartInitTransmit(void){
    TXSTAbits.TXEN = 1;                                                         // HABILITAR TRANSMISOR
    TXSTAbits.SYNC = 0;                                                         // ACTIVACION DE MODO ASINCRONO
    RCSTAbits.SPEN = 1;                                                         // HABILITAR EL MODULO DE TRANSMISION DE DATOS
    TXSTAbits.TX9 = 0;                                                          // ENVIO DE DATOS EN MODO DE 8
    
    // CONFIGURACION DE RAZON DE BAUDIOS (BAUDRATE) DE 9600 CON FOSC = 1MHz
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 1;
    SPBRG = 25;
    SPBRGH = 0;
    
    // CONFIGURACIONES PARA RECIBIR DATOS
    RCSTAbits.CREN = 1;
    PIE1bits.RCIE = 1;
}

// ENVIO DE UN DATO DE 8 BITS
void usartDataWrite(uint8_t msg){
    __delay_ms(1000);                                                           // DELAY PRE TRANSMISION
    if (PIR1bits.TXIF){                                                         // REVISAR SI SE PUEDE ENVIAR UN DATO
        TXREG = msg;                                                            // ENVIAR DATO DE 8 BITS
    }
}

//ENVIO DE DATOS EN FORMATO STRING
void uPrint(char *string){                                                      // CONVERTIR STRIGN A PUNTERO
    while(*string != '\0'){                                                     // REVISAR QUE EL VALOR DEL PUNTERO NO SEA NULO
        while(TXIF != 1);                                                       // SI NO ES NULO REVISAR QUE LA BANDERA DE INTERRUPCION ESTE ACTIVADA
        TXREG = *string;                                                        // SI ESTA ACTIVADA CARGAR CARACTER DEL PUNTERO A LA CONSOLA
        *string++;                                                              // INCREMENTAR DE POSICION EN EL PUNTERO
    }
}