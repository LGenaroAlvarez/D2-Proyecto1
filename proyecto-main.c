/*
 * File:   proyecto-main.c
 * Authors: Luis Genaro Alvarez Sulecio y Luis Alejandro Dardon Rivera
 * Description: Codigo para el control de motores segun datos recibidos del PIC principal mediante comunicacion USART.
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

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
/*LIBRERIAS INCLUIDAS*/
#include "osc_config.h"
#include "tmr0_config.h"
#include "USART.h"
/*===================*/

#define _XTAL_FREQ 1000000


// VARIABLES GLOBALES
uint8_t Fosc = 1;                                                               // SELECCIONAR FRECUENCIA DE OSCILADOR
int PS_val = 16;                                                                // VALOR DEL PRESCALER DEL TMR0
uint8_t cont = 0;                                                               // VALOR DEL CONTADOR DEL TMR0

// BANDERAS DE RECEPCION DE DATOS
uint8_t sensor_flag = 0;                                                        // DATOS RECIBIDOS POR COMUNICACION USART
uint8_t HS_flag = 0;                                                            // DATOS RECIBIDOS DEL SENSOR DE HUMEDAD
uint8_t spst_flag = 0;                                                          // DATOS RECIBIDOS DEL PUSHBUTTON
uint8_t flagdc =0;                                                              // DATOS RECIBIDOS DEL SENSOR DE TEMPERATURA (PARA EL MOTOR DC)

// VARIABLES PARA LECTURA USART
uint8_t frerr = 0;
uint8_t overr = 0;

// PROTOTIPO DE FUNCIONES
void setup(void);                                                               // FUNCION PARA CONFIGURACIONES I/O E INTERRUPCIONES
void servoRotate0(void);                                                        // FUNCION PARA GIRAR SERVO A 0°
void servoRotate180(void);                                                      // FUNCION PARA GIRAR SERVO A 180°

//INTERRUPCIONES
void __interrupt() isr(void){
    
    // INTERRUPCION DEL TMR0 PARA GENERAR PULSOS DE 20mS
    if (T0IF){
        cont++;                                                                 // CONTEO CON TIMER0
        if (spst_flag == 51){                                                   // REVISAR SI EL PUSHBUTTON DEL MOTOR STEPPER HA SIDO PRESIONADO
            if (cont == 1){                                                     // SI LA CUENTA ES IGUAL A 1
                PORTB = 0b00000010;                                             // ENCENDER EL PRIMER PIN DEL MOTOR STEPPER (RB1)
            }
            else if(cont == 2){                                                 // SI LA CUENTA ES IGUAL A 2
                PORTB = 0b00000100;                                             // ENCENDER EL PRIMER PIN DEL MOTOR STEPPER (RB2)
            }
            else if(cont == 3){                                                 // SI LA CUENTA ES IGUAL A 3
                PORTB = 0b00001000;                                             // ENCENDER EL PRIMER PIN DEL MOTOR STEPPER (RB3)

            }
            else if(cont == 4){                                                 // SI LA CUENTA ES IGUAL A 4
                PORTB = 0b00010000;                                             // ENCENDER EL PRIMER PIN DEL MOTOR STEPPER (RB4)
                cont = 0;                                                       // REINICIO DE LA CUENTA
            }
        }
        if (cont == 4){
            cont = 0;
        }
        tmr0Reset();                                                            // REINICIO DEL TMR0
    }
    
    // INTERRUPCION PARA RECEPCION DE VALORES DE LOS SENSORES
    if (RCIF){
        frerr = RCSTAbits.FERR;                                                 // REVISAR BANDERAS DE ERROR
        overr = RCSTAbits.OERR;                                                 //      |       |       |
        sensor_flag = RCREG;                                                    // LECTURA DE LOS DATOS RECIBIDOS POR TRANSMISION USART
        RCREG = 0;                                                              // LIMPIAR REGISTRO DE TRANSMISION DE DATOS USART
        if (sensor_flag <= 49){                                                 // REVISAR SI LOS DATOS RECIBIDOS CORRESPONDEN AL SENSOR DE HUMEDAD
            HS_flag = sensor_flag;                                              // CARGAR VALOR RECIBIDO A LA BANDERA DEL SENSOR DE HUMEDAD
        }
        else if (sensor_flag >= 50 && sensor_flag < 52){                        // REVISAR SI LOS DATOS RECIBIDOS CORRESPONDEN AL PUSHBUTTON
            spst_flag = sensor_flag;                                            // CARGAR VALOR RECIBIDO A LA BANDERA DEL PUSHBUTTON
        }
        else if (sensor_flag >= 52 && sensor_flag < 54){                        // REVISAR SI LOS DATOS RECIBIDOS CORRESPONDEN AL SENSOR DE HUMEDAD
            flagdc = sensor_flag;                                               // CARGAR VALOR RECIBIDO A LA BANDERA DEL MOTOR DC
        }
        else {
            sensor_flag = 0;                                                    // LIMPIAR BANDERA DE RECEPCION DE DATOS
        }
    }
}

void main(void) {
    setup();                                                                    // INICIALIZAR CONFIGURACIONES I/O E INTERRUPCIONES
    initOscFreq(Fosc);                                                          // INICIALIZAR CONFIGURACIONES DEL OSCILADOR A 1MHz
    initTmr0(PS_val);                                                           // INICIALIZAR CONFIGURACIONES DEL TMR0
    usartInitTransmit();                                                        // INICIALIZAR CONFIGURACIONES DE LA COMUNICACION USART
    
    /*==========================================================================
     =================================LOOP======================================
     =========================================================================*/
    while(1){
        if (HS_flag == 48){                                                     // SI LA BANDERA DEL SENSOR DE HUMEDAD ES IGUAL A 0
            servoRotate0();                                                     // MANTENER EL SERVO EN 0 GRADOS
        }
        else if (HS_flag == 49){                                                // SI LA BANDERA DEL SENSOR DE HUMEDAD ES IGUAL A 1
            servoRotate180();                                                   // MANTENER EL SERVO EN 180 GRADOS
        }
        if (flagdc == 52){                                                      // SI LA BANDERA DEL SENSOR DE TEMPERATURA ES IGUAL A 0
            PORTDbits.RD1 = 0;                                                  // MANTENER EL PIN RD1 EN 0 (MOTOR DC APAGADO)          
        }
        else if (flagdc == 53){                                                 // SI LA BANDERA DEL SENSOR DE TEMPERATURA ES IGUAL A 1
            PORTDbits.RD1 = 1;                                                  // MANTENER EL PIN RD1 EN 1 (MOTOR DC ENCENDIDO)
        }
    }
    /*========================================================================*/
    return;
}

void setup(void){
    //IO CONFIG
    ANSEL = 0;
    ANSELH = 0;
    
    // CONFIGURACION DE ENTRADAS Y SALIDAS
    TRISA = 0;
    TRISB = 0;
    TRISD = 0;
    
    // LIMPIEZA DE PUERTOS
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

// FUNCION PARA GIRAR SERVO A 0 GRADOS
void servoRotate0(void)
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    RD2 = 1;
    __delay_us(500);
    RD2 = 0;
    __delay_us(19500);
  }
}

// FUNCION PARA GIRAR SERVO A 180 GRADOS
void servoRotate180(void)
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    RD2 = 1;
    __delay_us(2200);
    RD2 = 0;
    __delay_us(17800);
  }
}