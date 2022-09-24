/* 
 * File                 : USART.h
 * Author               : L. Genaro Alvarez Sulecio
 * Comments             : Libreria para comunicacion USART asincrona entre dos dispositivos
 * Revision history     : 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef USART_COM
#define	USART_COM

#include <xc.h> // include processor files - each processor file is guarded.
#include <stdio.h>
#include <stdint.h>
//#include <pic16f887.h>

// PARA INICIALIZAR LA COMUNICACION USART EN MODO DE TRANSMISOR
void usartInitTransmit(void);
// ENVIO DE UN DATO DE 8 BITS
void usartDataWrite(uint8_t msg);
// ENVIO DE DATOS EN FORMATO STRING
void uPrint(char *string);
#endif	/* USART_COM */

