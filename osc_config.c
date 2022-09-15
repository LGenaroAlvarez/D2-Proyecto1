/*
 * File:   osc_config.c
 * Author: Ing. Luis Genaro Alvarez Sulecio
 *
 * Created on July 22, 2022, 8:49 AM
 */

#include "osc_config.h"
#include <xc.h>
#include <stdio.h>
#include <stdint.h>

void initOscFreq (uint8_t freq) {
    switch (freq){
        case 1:
            OSCCONbits.IRCF = 0b100;
            break;
        case 2:
            OSCCONbits.IRCF = 0b101;
            break;
        case 4:
            OSCCONbits.IRCF = 0b110;
            break;
        case 8:
            OSCCONbits.IRCF = 0b111;
            break;
        default:
            OSCCONbits.IRCF = 0b110;
            break;
    }
    OSCCONbits.SCS = 1;
}
