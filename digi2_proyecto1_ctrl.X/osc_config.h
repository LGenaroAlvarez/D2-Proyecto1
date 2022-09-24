/* 
 * File:   osc_config.h
 * Author: Ing. Luis Genaro Alvarez Sulecio
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OSC_CONFIG_H
#define	__OSC_CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdio.h>
#include <stdint.h>

/*PROTOTIPO DE FUNCION*/
void initOscFreq (uint8_t freq);

#endif	/* __OSC_CONFIG_H */

