/*
This is a power measurement program by counting the led blink of the power reader.
Power reader is a electronics equipment that its leds blinks every time 1 watt comsumed.
Idea: Using the light sensor to catch the led signal and count how many time it blink in one hour.
This code runs on NUCLEO-L152RE board
Using interrupt for PB15(D4), ADC input PA0 (A0), digital output PA10 (D2). D2 connect to D4.
Uart 9600 BAUD and float print with sprintf
*/

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>

/* ===================== GLOBAL ===================== */

volatile uint16_t count = 0;


/* ===================== FUNCTION PROTOTYPES ===================== */

void USART2_Init(void);
void USART2_write(char data);
void USART2_intWrite(int t);
void USART2_bufWrite(char *buf);

void timer5_init();
void adc_A0_init();
int read_A0();
void pa10_init();
void EXTI9_5_init();
void delay_Ms(int delay);

/* ===================== MAIN ===================== */

int main(void)
{
    __disable_irq();
    SetSysClock();
    SystemCoreClockUpdate();
    USART2_Init();
    timer5_init();
    adc_A0_init();
    pa10_init();
    EXTI9_5_init();

    __enable_irq();

	uint16_t li2;
    while (1)
    {
        li2 = read_A0();

        if (li2 < 2250) {
            GPIOA->ODR &= ~(1 << 10);
        }
        else if (li2 > 2850) {
            GPIOA->ODR |= (1 << 10);
        }

        int ti = TIM5->CNT;

        if (ti >= 3600000)
        {
            USART2_bufWrite("The last hour consumed: ");
            USART2_intWrite(count);
            USART2_bufWrite(" watt\n\r");

            count = 0;
            TIM5->CNT = 0;
        }
    }

    return 0;
}

/* ===================== TIMER ===================== */

void timer5_init()
{
    RCC->APB1ENR |= (1 << 3);
    TIM5->PSC = 32000 - 1;
    TIM5->ARR = 3600000 - 1;
    TIM5->CR1 = 1;
}

/* ===================== EXTERNAL INTERRUPT ===================== */

void EXTI9_5_init()
{
    RCC->AHBENR |= (1 << 1);        // Enable GPIOB
    GPIOB->MODER &= ~(0x03 << 10);   // PB5 input
    GPIOB->PUPDR &= ~(0x03 << 10);
    GPIOB->PUPDR |= (0x01 << 10);    // Pull-up

    RCC->APB2ENR |= (1 << 0);        // Enable SYSCFG

    SYSCFG->EXTICR[1] &= ~(0x0F << 4);
    SYSCFG->EXTICR[1] |= (0x01 << 4);

    EXTI->IMR |= (1 << 5);
    EXTI->FTSR |= (1 << 5);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void)
{
    if ((GPIOB->IDR & (1 << 5)) == 0)
    {
        count++;
    }

    EXTI->PR = (1 << 5);
}

/* ===================== GPIO ===================== */

void pa10_init()
{
    RCC->AHBENR |= (1 << 0);
    GPIOA->MODER &= ~(0x03 << 20);
    GPIOA->MODER |= (1 << 20);
}

/* ===================== ADC ===================== */

void adc_A0_init()
{
    RCC->AHBENR |= (1 << 0);
    GPIOA->MODER |= (0x03 << 0);

    RCC->APB2ENR |= (1 << 9);

    ADC1->CR2 &= ~(1 << 1);
    ADC1->SMPR3 |= (0x07 << 0);
    ADC1->CR1 &= ~(0x03 << 24);
}

int read_A0()
{
    int result = 0;

    ADC1->SQR5 = 0;
    ADC1->CR2 |= (1 << 0);
    ADC1->CR2 |= (1 << 30);

    while (!(ADC1->SR & (1 << 1)));

    result = ADC1->DR;

    ADC1->CR2 &= ~(1 << 0);

    return result;
}

/* ===================== USART ===================== */

void USART2_Init(void)
{
    RCC->APB1ENR |= (1 << 17);
    RCC->AHBENR |= (1 << 0);

    GPIOA->AFR[0] = 0x00000700;
    GPIOA->AFR[0] |= 0x00007000;

    GPIOA->MODER |= 0x00000020;
    GPIOA->MODER |= 0x00000080;

    USART2->BRR = 0x00000D05;
    USART2->CR1 = (1 << 3);
    USART2->CR1 |= (1 << 2);
    USART2->CR1 |= (1 << 13);
}

void USART2_write(char data)
{
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = data;
}

void USART2_bufWrite(char *buf)
{
    while (*buf)
    {
        USART2_write(*buf++);
    }
}

void USART2_intWrite(int t)
{
    char buf[100];
    sprintf(buf, "%d", t);

    for (int i = 0; buf[i] != '\0'; i++)
    {
        USART2_write(buf[i]);
    }
}
void delay_Ms(int delay)
{
	int i=0;
	for(; delay>0;delay--)
		for(i=0;i<2460;i++); //measured with oscilloscope
}
