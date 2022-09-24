/*
 * File:   adc_config.c
 * Author: Ing. Luis Genaro Alvarez Sulecio
 *
 * Created on July 22, 2022, 10:05 AM
 */

#ifndef _XTAL_FREQ
#define	_XTAL_FREQ 1000000
#endif

#include "adc_config.h"
#include <xc.h>
#include <stdio.h>
#include <stdint.h>

/* CONFIGURACION Y HABILITACION DEL ADC CON SELECCION DE FOSC Y VREF */
void initAdc (uint8_t adc_s, uint8_t vref_pos, uint8_t vref_neg) {
    PIR1bits.ADIF = 0;              // LIMPIEZA DE BANDERA DE INTERRUPCION DE ADC
    PIE1bits.ADIE = 1;              // HABILITAR INTERRUPCION DE ADC
    
    ADCON1bits.VCFG0 = vref_pos;    // SI vref_pos=0 > REFERENCIA INTERNA
    ADCON1bits.VCFG1 = vref_neg;    // SI vref_neg=0 > REFERENCIA INTERNA
    ADCON1bits.ADFM = 1;            // JUSTIFICADO A LA DERECHA
    ADCON0bits.ADON = 1;            // ACTIVAR ADC
    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
    
    /* SELECCION DE RELOJ DE CONVERSION */
    switch(adc_s){
        case 2: // FOSC/2 (UNICAMENTE PARA FOSC = 1MHz)
            ADCON0bits.ADCS = 0b00;
            break;
        case 8: // FOSC/8 (UNICAMENTE PARA FOSC = 4MHz)
            ADCON0bits.ADCS = 0b01;
            break;
        case 32: // FOSC/32 (UNICAMENTE PARA FOSC >= 8MHz)
            ADCON0bits.ADCS = 0b10;
            break;
        case 64: // FRC (UTILIZABLE EN CUALQUIER FRECUENCIA)
            ADCON0bits.ADCS = 0b11;
            break;
    }
}

/* INICIALIZAR LA CONVERSION EN EL CANAL SELECIONADO */
void adcGo(uint8_t channel) {
    ADCON0bits.ADCS = channel;      // SELECCION DE CANAL
    if (ADCON0bits.GO == 0){        // REVISAR SI SE HA INICIADO LA CONVERSACION
        ADCON0bits.GO = 1;          // INICIALIZAR CONVERSION
    }
}

/* INICIALIZAR LA CONVERSION CON LA CANTIDAD DE CANALES SELECCIONADOS*/
void Adc_Channel_Qty(uint8_t chs_qty){
    switch(chs_qty){
        case 1:
            if (ADCON0bits.GO == 0){        // REVISAR SI SE HA INICIADO LA CONVERSACION
                ADCON0bits.CHS = 0;
                __delay_us(40);
                ADCON0bits.GO = 1;          // INICIALIZAR CONVERSION
            }
            break;
        case 2:
            if (ADCON0bits.GO == 0){            // REVISAR SI EL ADC ESTA ENCENDIDO
                if (ADCON0bits.CHS == 0){           // REVISAR SI SE ENCUENTRA EN CANAL ANALOGICO 0
                    ADCON0bits.CHS = 1;             // CAMBIO A CANAL ANALOGICO 1
                    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
                }
                else if (ADCON0bits.CHS == 1){      // REVISAR SI SE ENCUENTRA EN CANAL ANALOGICO 1
                    ADCON0bits.CHS = 0;             // CAMBIO A CANAL ANALOGICO 0
                    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
                }
                __delay_us(40);                     // TIEMPO DE ESTABILIZACION
                ADCON0bits.GO = 1;                  // INICIADO DE CONVERSION
            }
            break;
        case 3:
            if (ADCON0bits.GO == 0){                // REVISAR SI EL ADC ESTA ENCENDIDO
                if (ADCON0bits.CHS == 0){           // REVISAR SI SE ENCUENTRA EN CANAL ANALOGICO 0
                    ADCON0bits.CHS = 1;             // CAMBIO A CANAL ANALOGICO 1
                    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
                }
                else if (ADCON0bits.CHS == 1){      // REVISAR SI SE ENCUENTRA EN CANAL ANALOGICO 1
                    ADCON0bits.CHS = 2;             // CAMBIO A CANAL ANALOGICO 2
                    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
                }
                else if (ADCON0bits.CHS == 2){      // REVISAR SI SE ENCUENTRA EN CANAL ANALOGICO 2
                    ADCON0bits.CHS = 0;             // CAMBIO A CANAL ANALOGICO 0
                    __delay_us(40);                 // TIEMPO DE ESTABILIZACION
                }
                __delay_us(40);                     // TIEMPO DE ESTABILIZACION
                ADCON0bits.GO = 1;                  // INICIADO DE CONVERSION
            }
            break;
        default:
            break;
    }
}

/* REALIZAR CONVERSION Y REGRESAR RESULTADOS */
uint16_t adcRead (void) {
    while(ADCON0bits.GO){}              // ESPERAR A QUE TERMINE LA CONVERSION
    return ( (ADRESH << 8) + ADRESL);   // REGRESAR VALOR DE CONVERSION
}
    
/* INTERPOLACION DE VALORES */
unsigned short map(uint16_t val, uint16_t pot_min, uint16_t pot_max,
        unsigned short out_min, unsigned short out_max){

    return (unsigned short)(out_min+((float)(out_max-out_min)/(pot_max-pot_min))
            *(val-pot_min));}