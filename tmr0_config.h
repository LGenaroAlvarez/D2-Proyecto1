/* 
 * File:                tmr0_config.h
 * Author:              Ing. Luis Genaro Alvarez Sulecio
 * Revision history:    ULTIMA ACTUALIZACION EL 12/08/2022 | SE CAMBIO EL TIPO DE VARIABLE DE PRESCALER A TIPO "INT" PARA QUE ESTA PUEDA RECIBIR EL VALOR 256 PARA EL SWITCH-CASE
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TMR0_CONFIG_H
#define	__TMR0_CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdio.h>
#include <stdint.h>

// LIBRERIA CONFIGURADA PARA UN PERIODO DE 500us A UNA FRECUENCIA DE 1MHz CON PRESCALER DE 16
#define tmr0_val 178

/* PROTOTIPO DE FUNCIONES */
void initTmr0 (int prescaler);
void tmr0Reset(void);

#endif	/* __TMR0_CONFIG_H */

