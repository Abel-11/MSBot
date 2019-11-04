/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.77
        Device            :  PIC18F45K22
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#define LED                             LATC7 
#define PWMA                            LATE2
#define AIN2                            LATC2
#define AIN1                            LATC3
#define STBY                            LATC1
#define BIN1                            LATD0
#define BIN2                            LATD2
#define PWMB                            LATD1

#define MANO_DERECHA        0
#define MANO_IZQUIERDA      1

//#define KP      1
//#define KD      0
float KP = 0.8;
float KD = 10;
#define VELOCIDAD   750
#define MAXIMO_REGISTRO_ANALOGICO       1023
#define MINIMO_REGISTRO_ANALOGICO       0
#define MUESTRAS_CALIBRACION            30000
#define LECTURAS_POR_MUESTRA              5
void encender_motores();
void apagar_motores();
void mover_motor_A(signed int velocidad);
void mover_motor_B(signed int velocidad);
void parada();
void moverse(signed int velocidad_A, signed int velocidad_B);

void leer_sensores();
float calculo_PID(int valor_sensor);


int sensor_izquierdo;
int sensor_izquierdo_delantero;
int sensor_derecho_delantero;
int sensor_derecho;
int set_point;

bit flag_inicio;


/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();
    char mano;
    char inicio;
    int valor_sensor_izquierdo, valor_sensor_derecho;
    int valor_sensor_izquierdo_delantero, valor_sensor_derecho_delantero;
    char media_sensores;
    int correccion;
    int velocidad_motor_A, velocidad_motor_B;
    
    
    while(ADC_GetConversion(0) < 700)
    {
        if(PORTBbits.RB1 == 0)
            mano = MANO_DERECHA;
        else
            mano = MANO_IZQUIERDA;
            
    }
    
    for(inicio = 0; inicio < 16; inicio++)
    {
        LED =~ LED;
        __delay_ms(100);
    }
    
    flag_inicio = 1;
    
    while (1)
    {
        for(media_sensores = 0; media_sensores < LECTURAS_POR_MUESTRA; media_sensores++)
        {
            leer_sensores();
            if(mano == MANO_IZQUIERDA)
            {
                valor_sensor_izquierdo = valor_sensor_izquierdo + sensor_izquierdo;
                valor_sensor_izquierdo_delantero = valor_sensor_izquierdo_delantero + sensor_izquierdo_delantero;
            }else
            {
                valor_sensor_derecho = valor_sensor_derecho + sensor_derecho;
                valor_sensor_derecho_delantero = valor_sensor_derecho_delantero + sensor_derecho_delantero;
            }
        }
        if(mano == MANO_IZQUIERDA)
        {
            valor_sensor_izquierdo = valor_sensor_izquierdo / LECTURAS_POR_MUESTRA;
            valor_sensor_izquierdo_delantero = valor_sensor_izquierdo_delantero / LECTURAS_POR_MUESTRA;
        }else
        {
            valor_sensor_derecho = valor_sensor_derecho / LECTURAS_POR_MUESTRA;
            valor_sensor_derecho_delantero = valor_sensor_derecho_delantero / LECTURAS_POR_MUESTRA;
        }
        
        if(flag_inicio && mano == MANO_IZQUIERDA)
        {
            set_point = valor_sensor_izquierdo;
            flag_inicio = 0;
        }else if(flag_inicio && mano == MANO_DERECHA)
        {
            set_point = valor_sensor_derecho;
            flag_inicio = 0;
        }
        
        if(mano == MANO_IZQUIERDA)
        {
            correccion = calculo_PID(valor_sensor_izquierdo);
            velocidad_motor_A = VELOCIDAD + correccion;
            velocidad_motor_B = VELOCIDAD - correccion;
            moverse(velocidad_motor_A, velocidad_motor_B);
        }else
        {
            correccion = calculo_PID(valor_sensor_derecho);
            velocidad_motor_A = VELOCIDAD - correccion;
            velocidad_motor_B = VELOCIDAD + correccion;
            moverse(velocidad_motor_A, velocidad_motor_B);
        }
    }
}

void encender_motores()
{
    STBY = 1;
}

void apagar_motores()
{
    STBY = 0;
}

void mover_motor_A(signed int velocidad)
{
    if (velocidad > 1023)
        velocidad = 1023;
    else if (velocidad < -1023)
        velocidad = -1023;
    
    if(velocidad >= 0)
    {
        AIN1 = 1;
        AIN2 = 0;
    }else
    {
        AIN1 = 0;
        AIN2 = 1;
        velocidad = velocidad * (-1);
    }


    PWM5_LoadDutyValue(velocidad);
}

void mover_motor_B(signed int velocidad)
{
    if (velocidad > 1023)
        velocidad = 1023;
    else if (velocidad < -1023)
        velocidad = -1023;
    
    if(velocidad >= 0)
    {
        BIN1 = 1;
        BIN2 = 0;
    }else
    {
        BIN1 = 0;
        BIN2 = 1;
        velocidad = velocidad * (-1);
    }

    PWM4_LoadDutyValue(velocidad);
}

void parada()
{
    AIN1 = 0;
    AIN2 = 0;

    PWM5_LoadDutyValue(0);
    
    BIN1 = 0;
    BIN2 = 0;

    PWM4_LoadDutyValue(0);
}

void moverse(signed int velocidad_A, signed int velocidad_B)
{
    mover_motor_A(velocidad_A);
    mover_motor_B(velocidad_B);
}

float calculo_PID(int valor_sensor)
{
    float correccion;
    static float last_error;
    float valor_proporcional, valor_diferencial;
    float error;
    
    

    error = set_point - valor_sensor;
    
    valor_proporcional = ((float)error)*KP;
    valor_diferencial = (error - last_error) * KD;
    
    correccion = valor_proporcional + valor_diferencial;
    
   /* printf("error:%f      last error:%f   dig:%i", error, last_error  , valor_digital);
       printf("\r\n");*/
    last_error = error;

    return correccion;
}

void leer_sensores()
{
    int sensor[8];
    char numero_sensor;
    ADC_GetConversion(11);
    ADC_GetConversion(13);
    ADC_GetConversion(11);
    ADC_GetConversion(13);
  
    sensor_izquierdo = sensor[0] = ADC_GetConversion(0);
    sensor_izquierdo_delantero = sensor[1] = ADC_GetConversion(1);
    sensor_derecho_delantero = sensor[2] = ADC_GetConversion(2);
    sensor_derecho = sensor[3] = ADC_GetConversion(3);

   /* for(numero_sensor = 0; numero_sensor < 4; numero_sensor++)
    {
        printf("%i   ", sensor[numero_sensor]);
       
    } printf("\r\n");*/
   
}

/**
 End of File
*/