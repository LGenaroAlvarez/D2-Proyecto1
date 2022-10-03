/*
 * File:   sensor_ctrl_main.c
 * Authors: Luis Genaro Alvarez Sulecio y Luis Alejandro Dardón Rivera
 * Description: Codigo para el control de una red de sensores i2C, analogicos y digitales. Cuenta con comunicacion USART para transmitir datos a un PIC secundario. 
 * Adicionalmente, cuenta con una pantalla LCD que es capaz de mostrar dos grupos de datos de manera individual intercambiando entre grupo al presionar un boton.
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
#include <conio.h>
#include <stdint.h>
#include <pic16f887.h>
/*LIBRERIAS INCLUIDAS*/
#include "osc_config.h"
#include "USART.h"
#include "adc_config.h"
#include "LCD4b.h"
#include "I2C.h"
/*===================*/
#define _XTAL_FREQ 1000000

// DEFINICIONES PARA LOS SENSORES
#define WATER_VAL 219                                                           // VALOR LEIDO POR SENSOR DE HUMEDAD EN AGUA
#define AIR_VAL 474                                                             // VALOR LEIDO POR SENSOR DE HUMEDAD AL AIRE
#define I2C_SPEED 100000                                                        // VELOCIDAD DE TRANSFERENCIA DE DATOS POR I2C
#define ADDRESS 0b1001000                                                       // DIRECCION DEL SENSOR DE TEMPERATURA
#define READ 0b0                                                                // VALOR DE LECTURA PARA SENSORES
#define WRITE 0b1                                                               // VALOR DE ESCRITURA PARA SENSORES

// VARIABLES GLOBALES
uint8_t Fosc = 1;                                                               // SELECCIONAR FRECUENCIA DE OSCILADOR
int i = 0;                                                                      // VARIABLE ALEATORIA

// VARIABLES PARA SENSOR DE HUMEDAD (HS = HUMIDITY SENSOR)
uint16_t HS_val = 0;                                                            // LECTURA INICIAL DEL SENSOR DE HUMEDAD
uint8_t HS_percent = 0;                                                         // PORCENTAJE DE HUMEDAD SEGUN VALOR DEL SENSOR
char HS_str[10];                                                                // BUFFER PARA OPERACION SPRINTF (PORCENTAJE)
char HS_flag_str[10];                                                           // BUFFER PARA OPERACION SPRINTF (VALOR CRUDO)
uint8_t HS_flag = 0;                                                            // BANDERA DE ESTADO DE SENSOR DE HUMEDAD

// VARIABLES PARA PUSHBUTTON
uint8_t spst = 0;                                                               // VARIABLE DE ESTADO DEL PUSHBUTTON 1 (SENSOR DIGITAL)
uint8_t disp = 0;                                                               // VARIABLE DE ESTADO DEL PUSHBUTTON 2 (CAMBIO DE DISPLAYS)
uint8_t spst_flag = 0;                                                          // BANDERA PARA INDICARLE AL SEGUNDO PIC EL ESTADO DEL SENSOR DIGITAL 
char spst_str[10];                                                              // BUFFER PARA ALMACENAR BANDERA COMO STRING PARA ENVIO POR USART

// VARIABLES PARA EL SENSOR DE TEMPERATURA
uint16_t cont1=0x00, data = 0, response = 0;                                    // TRANSMISION Y RECEPCION DE DATOS I2C
uint16_t temperatura = 0;                                                       // ALMACENAMIENTO DE DATOS DE TEMPERATURA LEIDOS EN BINARIO
int temp_dec = 0;                                                               // ALMACENAMIENTO DE DATOS DE TEMPERATURA EN DECIMAL
uint8_t temp_flag = 0;                                                          // BANDERA DE ESTADO DE TEMPERATURA PARA INDICAR AL SEGUNDO PIC LA TEMPERATURA OBTENIDA
char temp_str[10];                                                              // BUFFER PARA CONVERTIR DATOS A STRING Y MOSTRARLOS EN LA LCD

// VARIABLES PARA EL RTC
unsigned short RTC_sec = 0;                                                     // ALMACENAMIENTO DE SEGUNDOS DEL RTC
unsigned short RTC_min = 0;                                                     // ALMACENAMIENTO DE MINUTOS DEL RTC
unsigned short RTC_hor = 0;                                                     // ALMACENAMIENTO DE HORAS DEL RTC
char RTC_sec_str[10];                                                           // BUFFER PARA CONVERTIR SEGUNDOS A STRING Y MOSTRARLOS EN LA LCD
char RTC_min_str[10];                                                           // BUFFER PARA CONVERTIR MINUTOS A STRING Y MOSTRARLOS EN LA LCD
char RTC_hor_str[10];                                                           // BUFFER PARA CONVERTIR HORAS A STRING Y MOSTRARLOS EN LA LCD

// PROTOTIPO DE FUNCIONES
void setup(void);                                                               // FUNCION DE CONFIGURACIONES I/O
int percent(uint16_t val, uint16_t water_val, uint16_t air_val);                // FUNCION PARA CONVERTIR VALOR CRUDO A PORCENTAJE (SENSOR DE HUMEDAD)
uint8_t binTodec(uint8_t num);                                                  // FUNCION PARA CONVERTIR VALOR EN BINARIO A DECIMAL

// INTERRUPCIONES
void __interrupt() isr(void){
    /*==========================================================================
     *===========INTERRUPCION PARA LA LECTURA DEL SENSOR DE HUMEDAD=============
     =========================================================================*/
    if (ADIF){
        if (ADCON0bits.CHS == 0){                                               // DETERMINAR SI FUE INTERRUPCION EN EL PRIMER CANAL
            HS_val = adcRead();                                                 // LEER EL VALOR ENTREGADO POR EL SENSOR
            HS_percent = percent(HS_val, WATER_VAL, AIR_VAL);                   // CONVERTIR VALOR DEL SENSOR A PORCENTAJE
            sprintf(HS_str, "%d", HS_percent);                                  // INT TO STRING PARA PORCENTAJE            
        }
        PIR1bits.ADIF = 0;                                                      // LIMPIEZA DE BANDERA DE INTERRUPCION ADC
    }
    /*========================================================================*/
    
    /*==========================================================================
     ==============INTERRUPCION POR CAMBIO DE ESTADO EN PUERTO B================
     =========================================================================*/
    if (RBIF){
        // PUSHBUTTON PARA CONTROL DE MOTOR STEPPER
        if (!PORTBbits.RB0){                                                    // REVISAR SI EL PRIMER PUSHBUTTON HA SIDO PRESIONADO
            while(!PORTBbits.RB0);                                              // ANTIRREBOTES PARA PRIMER PUSHBUTTON
            spst = !spst;                                                       // CAMBIO DE ESTADO DE VARIABLE DE BOTON PARA DETERMINAR EL ESTADO DEL PUSHBUTTON
        }
        // PUSHBUTTON PARA CAMBIO DE DISPLAYS
        if (!PORTBbits.RB1){                                                    // REVISAR SI EL SEGUNDO PUSHBUTTON HA SIDO PRECIONADO
            while(!PORTBbits.RB1);                                              // ANTIRREBOTES PARA SEGUNDO PUSHBUTTON
            Lcd_Clear();                                                        // LIMPIEZA DEL DISPLAY PARA MOSTRAR NUEVOS DATOS
            disp = !disp;                                                       // CAMBIO DE ESTADO DE VARIABLE DE DISPLAY PARA DETERMINAR EL ESTADO DEL PUSHBUTTON
        }
        INTCONbits.RBIF = 0;                                                    // LIMPIAR BANDERA DE INTERRUPCION EN PUERTO B
    }
}
/*============================================================================*/

// CODIGO PRINCIPAL
void main(void) {
    setup();                                                                    // CONFIGURACION I/O Y DE INTERRUPCIONES
    initOscFreq(Fosc);                                                          // INICIALIZACION DEL OSCILADOR A 1MHz
    usartInitTransmit();                                                        // INICIALIZACION DE LA TRANSMISION USART
    initAdc(64,0,0);                                                            // INICIALIZACION DEL ADC
    Lcd_Init();                                                                 // INICIALIZACION DE LA LCD
    I2C_Master_Init(115000);                                                    // INICIALIZACION DE LA TRANSMISION I2C
    /*==========================================================================
     *===================================LOOP===================================
     =========================================================================*/
    while(1){
        adcGo(0); 
        /*======================================================================
         ========================LECTURA DE SENSORES I2C========================
         =====================================================================*/
        
        // SENSOR DE TEMPERATURA
        I2C_Master_Start();                                                     // INICIALIZACION DE COMUNICACION I2C
        I2C_Master_Write(ADDRESS+WRITE);                                        // DIRECCION DEL SENSOR DE TEMPERATURA EN MODO ESCRITURA
        I2C_Master_Write(0x01);                                                 // ESCRITURA AL SENSOR DE TEMPERATURA PARA DATOS REQUERIDOS
        I2C_Master_Write(0x18);                                                 // |        |       |       |       |
        I2C_Master_RepeatedStart();                                             // REINICIO DE COMUNICACIONES
        I2C_Master_Write(0x91);                                                 // DIRECCION DEL SENSOR DE TEMPERATURA EN MODO DE LECTURA
        temperatura = I2C_Master_Read(0);                                       // LECTURA DE VALORES SOLICITADOS
        I2C_Master_Stop();                                                      // FINALIZACION DE COMUNICACION 12C
        __delay_ms(100);                                                        // TIEMPO DE ESTABILIZACION
        sprintf(temp_str, "%02d ", temperatura);                                // CONVERSION DE DATOS DE INT A STRING PARA DISPLAY EN LCD
        
        // SENSOR RTC
        I2C_Master_Start();                                                     // INICIALIZACION DE COMUNICACION I2C
        I2C_Master_Write(0xD0);                                                 // DIRECCION DEL SENSOR RTC EN MODO ESCRITURA
        I2C_Master_Write(0);                                                    // SOLICITUD DE DATOS REQUERIDOS DEL SENSOR
        I2C_Master_RepeatedStart();                                             // REINICIO DE COMUNICACIONES
        I2C_Master_Write(0xD1);                                                 // DIRECCION DEL SENSOR RTC EN MODO DE LECTURA
        RTC_sec = I2C_Master_Read(1);                                           // LECTURA DE SEGUNDOS
        RTC_min = I2C_Master_Read(1);                                           // LECTURA DE MINUTOS
        RTC_hor = I2C_Master_Read(0);                                           // LECTURA DE HORAS
        I2C_Master_Stop();                                                      // FINALIZACION DE COMUNICACION 12C
        __delay_ms(100);                                                        // TIEMPO DE ESTABILIZACION   
        RTC_sec = binTodec(RTC_sec);                                            // CONVERSION DE DATOS DE SEGUNDOS DE BINARIO A DECIMAL
        RTC_min = binTodec(RTC_min);                                            // CONVERSION DE DATOS DE MINUTOS DE BINARIO A DECIMAL
        RTC_hor = binTodec(RTC_hor);                                            // CONVERSION DE DATOS DE HORAS DE BINARIO A DECIMAL
        sprintf(RTC_sec_str, "%02d", RTC_sec);                                  // STRING TO INT PARA SEGUNDOS
        sprintf(RTC_min_str, "%02d", RTC_min);                                  // STRING TO INT PARA MINUTOS
        sprintf(RTC_hor_str, "%02d", RTC_hor);                                  // STRING TO INT PARA HORAS
        /**/
        
        /*======================================================================
         *==========CODIGO PARA DETERMINAR SI IRRIGAR O NO EL COMPOST===========
         =====================================================================*/
        if (HS_val >= 384){//HS% MENOR AL 35%                                   // SEGUN EL ESTUDIO REALIZADO POR HAUG (1993), EL PORCENTAJE OPTIMO DE HUMEDAD PARA
            HS_flag = 49;                           // ACTIVAR BANDERA HS       // LOGRAR UN COMPOSTAJE EXITOSO SE ENCUENTRA ENTRE EL 40% Y 60%. AHORA BIEN, OTROS ESTUDIOS
            sprintf(HS_flag_str, "%d", HS_flag);    // INT TO STRING            // HAN DEMOSTRADO QUE SE REQUIERE UN PORCENTAJE DE HUMEDAD MINIMO DEL 50% MIENTRAS QUE OTROS
        }                                                                       // INDICAN UN PORCENTAJE DE AL MENOS 25%. CONSIDERANDO QUE ALGUNOS DE ESTOS ESTUDIOS SE BASARON
        else if (HS_val <= 358){//HS% MAYOR AL 45%                              // EN EL COMPOSTAJE DE ESTIERCOL DE VACA, Y QUE EL COMPOSTERO URBANO NO REQUERIRA ESTABILIZAR DICHO
            HS_flag = 48;                           // DESACTIVAR BANDERA HS    // MATERIAL, SE ESTABLECIO UN PORCENTAJE OPTIMO ENTRE EL 35% Y 45%. ESTO CON TAL DE TOMAR EN CUENTA
            sprintf(HS_flag_str, "%d", HS_flag);    // INT TO STRING            // CUALQUIER DIFERENCIA EN EL FLUJO DE AGUA UTILIZADO EN LA IRRIGACION. INFORMACION DE:
        }                                                                       //https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4852240/#:~:text=Many%20previous%20studies%20have%20suggested,40%25)%20limits%20microbial%20activity.
        /*====================================================================*/
        
        /*======================================================================
         =============ASIGNACION DE BANDERAS PARA TRANSMISION USART=============
         =====================================================================*/
        if (spst == 0){                                                         // SI EL BOTON ESTA LIBERADO
            spst_flag = 50;                                                     // DESACTIVAR BANDERA DEL PUSHBUTTON
        }
        else if (spst == 1){                                                    // SI EL BOTON ESTA PRESIONADO
            spst_flag = 51;                                                     // ACTIVAR BANDERA DEL PUSHBUTTON
        }
        
        if (temperatura <= 25){                                                 // SI LA TEMPERATURA ES MENOR A 25°        
            temp_flag = 52;                                                     // DESACTIVAR BANDERA DEL SENSOR DE TEMPERATURA
        }
        else if (temperatura > 26 && temperatura < 32){                         // SI LA TEMPERATURA ES MENOR QUE 32° Y MAYOR A 26°
            temp_flag = 53;                                                     // ACTIVAR LA BANDERA DEL SENSOR DE TEMPERATURA
        }
        
        // SI NO SE HA PRESIONADO EL BOTON, MOSTRAR HUMEDAD, TEMPERATURA Y ESTADO DEL PUSHBUTTON
        if (disp == 0){
        /*======================================================================
         *=======CODIGO PARA MOSTRAR DATOS DEL SENSOR DE HUMEDAD EN LCD=========
         =====================================================================*/
            Lcd_Set_Cursor(0,0);
            Lcd_Write_String("Hum:");
            if (HS_val < 466 & HS_val > 221){
                Lcd_Set_Cursor(1,0);
                Lcd_Write_String(" ");
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(HS_str);
            }
            else if (HS_val > 460){  
                Lcd_Set_Cursor(1,0);
                Lcd_Write_String(" ");
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("00");
            }
            else if (HS_val < 220){
                Lcd_Set_Cursor(1,0);
                Lcd_Write_String("100");
            }
            Lcd_Set_Cursor(1,3);
            Lcd_Write_String("%");    
        /*====================================================================*/

        /*======================================================================
         *==========CODIGO PARA MOSTRAR DATOS DEL PUSHBUTTON EN LA LCD==========
         =====================================================================*/
            Lcd_Set_Cursor(0,5);
            Lcd_Write_String("Bot:");
            if (spst == 0){                
                Lcd_Set_Cursor(1,5);
                Lcd_Write_String("Off");
            }
            else if (spst == 1){
                Lcd_Set_Cursor(1,5);
                Lcd_Write_String(" ");
                Lcd_Set_Cursor(1,6);
                Lcd_Write_String("On");
            }
        /*====================================================================*/

        /*======================================================================
         ==========CODIGO PARA MOSTRAR DATOS DE TEMPERATURA EN LA LCD===========
         =====================================================================*/
            Lcd_Set_Cursor(0,10);
            Lcd_Write_String("Temp:");            
            Lcd_Set_Cursor(1,12);
            Lcd_Write_String(" ");
            if (temperatura != 255){
                Lcd_Set_Cursor(1,10);
                Lcd_Write_String(temp_str);
            }
            else if (temperatura == 255){
                temp_flag = 52;
                Lcd_Set_Cursor(1,10);
                Lcd_Write_String("25");
            }
        }
        /*====================================================================*/
        
        /*======================================================================
         =======================MOSTRAR TIEMPO SEGUN BOTON======================
         =====================================================================*/
        else if (disp = 1){
            Lcd_Set_Cursor(0,6);
            Lcd_Write_String("Hora:");
            Lcd_Set_Cursor(1,6);
            Lcd_Write_String(RTC_hor_str);
            Lcd_Set_Cursor(1,8);
            Lcd_Write_String(":");
            Lcd_Set_Cursor(1,9);
            Lcd_Write_String(RTC_min_str);
        }
        /*====================================================================*/
        
        /*======================================================================
         *=======================ENVIO DE DATOS POR USART=======================
         =====================================================================*/
        usartDataWrite(HS_flag);                                                // ENVIO DE BANDERA DE ESTADO DEL SENSOR DE HUMEDAD
        usartDataWrite(spst_flag);                                              // ENVIO DE BANDERA DE ESTADO DEL PUSHBUTTON
        usartDataWrite(temp_flag);                                              // ENVIO DE BANDERA DE ESTADO DEL SENSOR DE TEMPERATURA
        /*====================================================================*/

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
    TRISB = 0b00000011;
    OPTION_REGbits.nRBPU = 0;                                                   // HABILITAR WEAK PULLUP EN PUERTO B
    WPUB = 0b00000011;                                                          // HABILITAR RESISTENCIA EN RB0 Y RB1
    IOCBbits.IOCB0 = 1;                                                         // HABILITAR INTERRUPCION EN CAMBIO PARA RB0
    IOCBbits.IOCB1 = 1;                                                         // HABILITAR INTERRUPCION EN CAMBIO PARA RB1
    
    TRISCbits.TRISC3 = 1;                                                       // HABILITAR PINES PARA COMUNICACION I2C
    TRISCbits.TRISC4 = 1;                                                       //      |       |           |           |
    TRISD = 0;                                                                  // LIMPIEZA DE PUERTOS
    PORTA = 0;                                                                  //      |       |
    PORTB = 0;                                                                  //      |       |
    PORTD = 0;                                                                  //      |       |
    
    //I2C CONFIG
    SSPADD = ((_XTAL_FREQ)/(4*I2C_SPEED)) - 1;                                  // 100 kHz
    SSPSTATbits.SMP = 1;                                                        // Velocidad de rotación
    SSPCONbits.SSPM = 0b1000;                                                   // I2C master mode, clock= Fosc/(4*(SSPADD+1))
    SSPCONbits.SSPEN = 1;                                                       // Habilitamos pines de I2C
    PIR1bits.SSPIF = 0;                                                         // Limpiamos bandera de interrupción de I2C
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;                                                         // ACTIVAR INTERRUPCIONES GLOBALES
    INTCONbits.PEIE = 1;                                                        // ACTIVAR INTERRUPCIONES EN PERIFERICOS
    INTCONbits.RBIE = 1;                                                        // ACTIVAR INTERRUPCIONES EN PUERTO B
    INTCONbits.RBIF = 0;                                                        // LIMPIAR BANDERA DE INTERRUPCIONES EN PUERTO B
}

// FUNCION PARA CONVERTIR VALOR CRUDO A PORCENTAJE
int percent(uint16_t val, uint16_t water_val, uint16_t air_val){                // val = valor crudo    water_val = limite inferior     air_val = limite superior
    return (int)(((air_val-val)*100)/(air_val-water_val));                      // [(limite superior - valor crudo)x100]/
}                                                                               //   (limite superior - limite inferior)

// FUNCION PARA CONVERTIR DATOS DE BINARIO A DECIMAL
uint8_t binTodec(uint8_t num){
    return((num >> 4) * 10 + (num & 0x0F));
}
