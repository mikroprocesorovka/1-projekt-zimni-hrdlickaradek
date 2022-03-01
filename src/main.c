#include "stm8s.h"
#include "milis.h"
#include "spse_stm8.h"
#include "stm8_hd44780.h"

/*#include "delay.h"*/
// #include <cstdint>
#include <stdio.h>
#include "stm8s_adc2.h"
//#include "uart1.h"

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define buzzer_PORT GPIOD
#define buzzer_PIN  GPIO_PIN_6
#define buzzer_HIGH   GPIO_WriteHigh(buzzer_PORT, buzzer_PIN)
#define buzzer_LOW  GPIO_WriteLow(buzzer_PORT, buzzer_PIN)
#define buzzer_REVERSE GPIO_WriteReverse(buzzer_PORT, buzzer_PIN)

#define BTN_PORT GPIOE
#define BTN_PIN  GPIO_PIN_4
#define BTN_PUSH (GPIO_ReadInputPin(BTN_PORT, BTN_PIN)==RESET) 

void delay_ms(uint16_t ms) {
    uint16_t  i;
    for (i=0; i<ms; i = i+1){
        _delay_us(250);
        _delay_us(248);
        _delay_us(250);
        _delay_us(250);
    }
}

void setup(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    GPIO_Init(buzzer_PORT, buzzer_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(BTN_PORT, BTN_PIN, GPIO_MODE_IN_FL_NO_IT);
    buzzer_HIGH;

    init_milis();

    lcd_init();

    // inicializace ADC //
    // na pinech/vstupech ADC_IN2 (PB4) a ADC_IN3 (PB5) vypneme vstupní buffer
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL4, DISABLE);
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL5, DISABLE);

    // při inicializaci volíme frekvenci AD převodníku mezi 1-4MHz při 3.3V
    // mezi 1-6MHz při 5V napájení
    // nastavíme clock pro ADC (16MHz / 4 = 4MHz)
    ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);

    // volíme zarovnání výsledku (typicky vpravo, jen vyjmečně je výhodné vlevo)
    ADC2_AlignConfig(ADC2_ALIGN_RIGHT);
    
    // nasatvíme multiplexer na některý ze vstupních kanálů
    ADC2_Select_Channel(ADC2_CHANNEL_4);
    // rozběhneme AD převodník
    ADC2_Cmd(ENABLE);
    // počkáme než se AD převodník rozběhne (~7us)
    ADC2_Startup_Wait();
}


int main(void)
{
    uint32_t time = 0;
    uint16_t ADCx;
    uint16_t voltage;
    uint16_t hodnota;

    char text[32];

    uint8_t bzucak = 1;

    setup();

    while (1) {

        if (milis() - time > 100) {
            time = milis();
            ADCx = ADC_get(ADC2_CHANNEL_4);
            voltage = ADCx * 3,3 / 1,024;
            hodnota = (voltage-400) / 19,5;
            lcd_gotoxy(0, 0);
            sprintf(text,"Teplota = %1u C",hodnota);
            lcd_puts(text);
        }

        if(hodnota < 10){
            if(bzucak == 1){                
                buzzer_LOW;
                delay_ms(1000);
                buzzer_HIGH;
            }
            bzucak = 0;
        }

        if(hodnota > 15){
            bzucak = 1;
        }

    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
