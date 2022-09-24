/*
 * File:   sensor_ctrl_main.c
 * Author: luisg
 *
 * Created on September 16, 2022, 12:05 PM
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
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <pic16f887.h>
#include "osc_config.h"
#include "tmr0_config.h"
#include "USART.h"
#include "adc_config.h"
#include "LCD4b.h"

#define _XTAL_FREQ 1000000

// DEFINICIONES PARA LOS SENSORES
#define WATER_VAL 219                                                           // EN AGUA
#define AIR_VAL 474                                                             // AL AIRE


// VARIABLES GLOBALES
uint8_t Fosc = 1;                                                               // SELECCIONAR FRECUENCIA DE OSCILADOR
int PS_val = 16;                                                                // VALOR DEL PRESCALER DEL TMR0
uint8_t cont = 0;                                                               // VALOR DEL CONTADOR DEL TMR0
uint8_t milis = 0;
uint8_t pulse = 0;

int i = 0;

// VARIABLES PARA MODULO USART
uint8_t send_cont = 0;
uint8_t msg_flag = 0;

// VARIABLES PARA SENSOR DE HUMEDAD (HS = HUMIDITY SENSOR)
uint16_t HS_val = 0;                                                            // LECTURA INICIAL DEL SENSOR DE HUMEDAD
uint8_t HS_percent = 0;                                                         // PORCENTAJE DE HUMEDAD SEGUN VALOR DEL SENSOR
char HS_str[10];                                                                // BUFFER PARA OPERACION SPRINTF (PORCENTAJE)
char HS_flag_str[10];                                                            // BUFFER PARA OPERACION SPRINTF (VALOR CRUDO)
uint8_t HS_flag = 0;                                                            // BANDERA DE ESTADO DE SENSOR DE HUMEDAD

// VARIABLES PARA PUSHBUTTON
uint8_t spst = 0;
uint8_t spst_flag = 0;
char spst_str[10];

// PROTOTIPO DE FUNCIONES
void setup(void);
void stepSet(uint8_t push, uint8_t set_pulse);                                  // FUNCION DE CONTROL DE MOTOR STEPPER
int percent(uint16_t val, uint16_t water_val, uint16_t air_val);                // FUNCION PARA CONVERTIR VALOR CRUDO A PORCENTAJE (SENSOR DE HUMEDAD)

// INTERRUPCIONES
void __interrupt() isr(void){
    /*==========================================================================
     *=============TMR0 PARA ENVIO DE DATOS USART Y PULSOS STEPPER==============
     =========================================================================*/    
    if (T0IF){
        send_cont++;
        milis++;
        if (milis == 4){
            pulse = !pulse;
            PORTDbits.RD6 = pulse;
            milis = 0;            
        }
//        // ENVIO DE DATOS CADA 1mS
//        if (send_cont == 100 & msg_flag == 0){                                    // SI msg_flag = 0, ENVIAR ESTADO DE BANDERA DEL SENSOR DE HUMEDAD
//            uPrint(HS_flag_str);
//            msg_flag = 1;
//            send_cont = 0;
//        }
//        else if (send_cont == 100 & msg_flag == 1){
//            //uPrint(spst_str);
//            msg_flag = 0;
//            send_cont = 0;
//        }
                
        tmr0Reset();
    }
    /*========================================================================*/
    
    /*==========================================================================
     *===========INTERRUPCION PARA LA LECTURA DEL SENSOR DE HUMEDAD=============
     =========================================================================*/
    if (ADIF){
        if (ADCON0bits.CHS == 0){
            HS_val = adcRead();                                                 // LEER EL VALOR ENTREGADO POR EL SENSOR
            HS_percent = percent(HS_val, WATER_VAL, AIR_VAL);                   // CONVERTIR VALOR DEL SENSOR A PORCENTAJE
            sprintf(HS_str, "%d", HS_percent);                                  // INT TO STRING PARA PORCENTAJE            
        }
        PIR1bits.ADIF = 0;                                                      // LIMPIEZA DE BANDERA DE INTERRUPCION ADC
    }
    /*========================================================================*/
    
    if (RBIF){
        if (!PORTBbits.RB0){
            while(!PORTBbits.RB0);
            spst = !spst;
            PORTDbits.RD7 = spst;
            spst_flag = spst+2;
            sprintf(spst_str, "%d", (spst_flag));
        }        
        INTCONbits.RBIF = 0;
    }
}

// CODIGO PRINCIPAL
void main(void) {
    setup();
    initOscFreq(Fosc);                                                          // INICIALIZACION DEL OSCILADOR A 1MHz
    initTmr0(PS_val);                                                           // INICIALIZACION DEL TMR0 A 500uS
    usartInitTransmit();                                                        // INICIALIZACION DE LA TRANSMISION USART
    initAdc(64,0,0);                                                            // INICIALIZACION DEL ADC
    Lcd_Init();                                                                 // INICIALIZACION DE LA LCD
    /*==========================================================================
     *===================================LOOP===================================
     =========================================================================*/
    while(1){
        adcGo(0);        
        /*======================================================================
         *==========CODIGO PARA DETERMINAR SI IRRIGAR O NO EL COMPOST===========
         =====================================================================*/
        if (HS_val >= 384){//HS% MENOR AL 35%                                   // SEGUN EL ESTUDIO REALIZADO POR HAUG (1993), EL PORCENTAJE OPTIMO DE HUMEDAD PARA
            HS_flag = 1;                            // ACTIVAR BANDERA HS       // LOGRAR UN COMPOSTAJE EXITOSO SE ENCUENTRA ENTRE EL 40% Y 60%. AHORA BIEN, OTROS ESTUDIOS
            sprintf(HS_flag_str, "%d", HS_flag);    // INT TO STRING            // HAN DEMOSTRADO QUE SE REQUIERE UN PORCENTAJE DE HUMEDAD MINIMO DEL 50% MIENTRAS QUE OTROS
        }                                                                       // INDICAN UN PORCENTAJE DE AL MENOS 25%. CONSIDERANDO QUE ALGUNOS DE ESTOS ESTUDIOS SE BASARON
        else if (HS_val <= 358){//HS% MAYOR AL 45%                              // EN EL COMPOSTAJE DE ESTIERCOL DE VACA, Y EL COMPOSTERO URBANO NO REQUERIRA ESTABILIZAR DICHO
            HS_flag = 0;                            // DESACTIVAR BANDERA HS    // MATERIAL, SE ESTABLECIO UN PORCENTAJE OPTIMO ENTRE EL 35% Y 45%. ESTO CON TAL DE TOMAR EN CUENTA
            sprintf(HS_flag_str, "%d", HS_flag);    // INT TO STRING            // CUALQUIER DIFERENCIA EN EL FLUJO DE AGUA UTILIZADO EN LA IRRIGACION. INFORMACION DE:
        }                                                                       //https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4852240/#:~:text=Many%20previous%20studies%20have%20suggested,40%25)%20limits%20microbial%20activity.
        /*====================================================================*/
        
        /*======================================================================
         *=======CODIGO PARA MOSTRAR DATOS DEL SENSOR DE HUMEDAD EN LCD=========
         =====================================================================*/
        Lcd_Set_Cursor(0,2);
        Lcd_Write_String("Humedad");
        if (HS_val < 466 & HS_val > 221){
            Lcd_Set_Cursor(1,2);
            Lcd_Write_String(" ");
            Lcd_Set_Cursor(1,3);
            Lcd_Write_String(HS_str);
        }
        else if (HS_val > 460){  
            Lcd_Set_Cursor(1,2);
            Lcd_Write_String(" ");
            Lcd_Set_Cursor(1,3);
            Lcd_Write_String("00");
        }
        else if (HS_val < 220){
            Lcd_Set_Cursor(1,2);
            Lcd_Write_String("100");
        }
        Lcd_Set_Cursor(1,5);
        Lcd_Write_String("%");    
        /*====================================================================*/
        
        /**/
        uPrint("A");
        __delay_ms(500);
        uPrint(HS_flag_str);
        __delay_ms(500);
        uPrint("B");
        __delay_ms(500);
        uPrint(spst_str);
        __delay_ms(500);
        /*====================================================================*/
//        while (i == 0){
//            uPrint("Conexion Exitosa");
//            i++;
//        }
    }
    /*========================================================================*/
    return;
}

// SETUP
void setup(void){
    //IO CONFIG
    ANSEL = 0b00000001;
    ANSELH = 0;
    
    // CONFIGURACION DE PUERTO A PARA ADC
    TRISA = 0b00000001;
    
    // CONFIGURACION DE PUERTO B PARA PUSHBUTTON
    TRISB = 0b00000001;
    OPTION_REGbits.nRBPU = 0;                                                   // HABILITAR WEAK PULLUP EN PUERTO B
    WPUB = 0b00000001;                                                          // HABILITAR RESISTENCIA EN RB0 
    IOCBbits.IOCB0 = 1;                                                         // HABILITAR INTERRUPCION EN CAMBIO PARA RB0
    
    TRISD = 0;
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;
}

void stepSet(uint8_t push, uint8_t set_pulse){
    if (push == 1){
        PORTCbits.RC0 = set_pulse;
    }
    else {
        PORTCbits.RC0 = 0;
    }
}

// FUNCION PARA CONVERTIR VALOR CRUDO A PORCENTAJE
int percent(uint16_t val, uint16_t water_val, uint16_t air_val){                // val = valor crudo    water_val = limite inferior     air_val = limite superior
    return (int)(((air_val-val)*100)/(air_val-water_val));                      // [(limite superior - valor crudo)x100]/
}                                                                               //   (limite superior - limite inferior)