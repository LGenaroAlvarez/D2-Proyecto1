/* 
 * File:   adc_config.h
 * Author: Ing. Luis Genaro Alvarez Sulecio
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _XTAL_FREQ
#define	_XTAL_FREQ 1000000
#endif

#ifndef __ADC_CONFIG_H
#define	__ADC_CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdio.h>
#include <stdint.h>

/* PROTOTIPO DE FUNCIONES */
void initAdc (uint8_t adcs, uint8_t vref_pos, uint8_t vref_neg);                // CONFIGURACION DEL ADC
void adcGo(uint8_t channel);                                                    // INICIALIZACION DE LA CONVERSION EN 1 CANAL ESPECIFICADO
void Adc_Channel_Qty(uint8_t chs_qty);                                          // FUNCION OPCIONAL, UTILIZADA EN EL CASO DE HABER MÁS DE 1 CANAL EN USO
uint16_t adcRead (void);                                                        // LECTURA DE VALORES DE LA CONVERSION DEL ADC

/* FUNCION PARA INTERPOLACION */
unsigned short map(uint16_t val, uint16_t pot_min, uint16_t pot_max,
        unsigned short out_min, unsigned short out_max);


#endif	/* __ADC_CONFIG_H */

