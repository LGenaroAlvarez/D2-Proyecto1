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
    //__delay_ms(100);                                                           // DELAY PRE TRANSMISION
    if (PIR1bits.TXIF && TXSTAbits.TRMT){                                                         // REVISAR SI SE PUEDE ENVIAR UN DATO
        TXREG = msg;                                                            // ENVIAR DATO DE 8 BITS
    }
}

/* PROTOCOLO DE ENVIO DE DATOS DE LOS SENSORES
 1. ESTABLECER LA CANTIDAD DE PAQUETES DE 8 BITS QUE SE ENVIARAN AL RECEPTOR (ESTA CANTIDAD DEBE COINCIDIR ENTRE EL EMISOR Y RECEPTOR)
 2. ENVIAR ADDRESS DEL SENSOR (CODIGO ASCII MAX 8BITS ASIGNADO POR EL USUARIO)
 3. ENVIAR PAQUETES DEL SENSOR SELECCIONADO UNO DESPUES DEL OTRO AL RECEPTOR
 4. */