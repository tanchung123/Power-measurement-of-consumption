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

/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */
void USART2_Init(void);
void USART2_write(char data);
char USART2_read();
void USART2_intWrite(int t);
void USART2_bufWrite(char *buf);
void delay_Ms(int delay);
//void adc_A1_init();
//int read_A1();
//static void bubbleShort5(int a[5]);
//void adc_B0_init();
//uint16_t read_B0();
//int abs(int a);
//void pow_int();
void timer5_init();
void adc_A0_init();
int read_A0();
void pa10_init();
void EXTI9_5_init();

volatile volatile uint16_t count=0, start=0;

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  /* Configure the system clock to 32 MHz and update SystemCoreClock */
	__disable_irq();
  SetSysClock();
  SystemCoreClockUpdate();
  USART2_Init();
  //pow_int();
  timer5_init();
  //adc_A1_init();
  //adc_B0_init();
  adc_A0_init();
  pa10_init();
  EXTI9_5_init();
  __enable_irq();
  /* Infinite loop */
  while (1)
  {
	li2=read_A0();
	if(li2<2250){
		GPIOA->ODR&=~(0x01<<10);
	}else if(li2>2850){
		GPIOA->ODR|=0x01<<10;
	}
	int ti=TIM5->CNT;
	if(ti>=3600000){
		USART2_bufWrite("The last hour consumed: ");
		USART2_intWrite(count);
		USART2_bufWrite("watt");
		USART2_write('\n');
		USART2_write('\r');
		count=0;
		TIM5->CNT=0;
	}
  }
	return 0;
}

void timer5_init(){
	RCC->APB1ENR|=0x01<<3;
	TIM5->PSC=32000-1;
	TIM5->ARR=5400000-1;
	TIM5->CR1=1; //timer 5 enable
}

/*void TIM3_IRQHandler(void){

}*/

void EXTI9_5_init(){
	RCC->AHBENR|=0x01<<1; //turn on GPIOB
	GPIOB->MODER&=~(0x03<<10); //set PB5 input
	GPIOB->PUPDR&=~(0x03<<10); //reset pulling
	GPIOB->PUPDR|=0x01<<10;	//pulling up
	RCC->APB2ENR|=0x01<<0; //turn syscfg
	SYSCFG->EXTICR[1]&=~(0x0f<<4);	//reset ext5
	SYSCFG->EXTICR[1]|=0x01<<4;	//set ext5 PB5
	EXTI->IMR|=0x02<<4;
	EXTI->FTSR|=0x02<<4;
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void){
	if((GPIOB->IDR&(2<<4))==0){
		count++;
	}
	USART2_intWrite(count);
	USART2_write('\n');
	USART2_write('\r');
	int time=TIM5->CNT;
	USART2_intWrite(time);
	USART2_bufWrite(" - ");
	USART2_intWrite(time/1000);
	USART2_bufWrite("s");
	USART2_write('\n');
	USART2_write('\r');
	EXTI->PR=0x02<<4;
}

void pa10_init(){
	RCC->AHBENR|=0x01<<0;	//turn on GPIOA
	GPIOA->MODER&=~(0x03<<20);	//reset PA10 output
	GPIOA->MODER|=0x01<<20;	//set PA10 output
}

void adc_A0_init(){	//ADC for lmt84
	RCC->AHBENR|=0x01<<0;	//turn GPIOA
	GPIOA->MODER|=0x03<<0;	//turn PA0 analog

	RCC->APB2ENR|=0x01<<9;	//turn ADC
	ADC1->CR2&=~0x01<<1;	//single mode
	ADC1->SMPR3|=0x07<<0;	//384 cycle for channel 1
	ADC1->CR1&=~0x03<<24;	//resolution 12 bit
}

int read_A0()	//ADC for lmt84
{
	int result=0;
	ADC1->SQR5=0;				//conversion sequence to ch0
	ADC1->CR2|=1;				//bit 0, ADC on/off (1=on, 0=off)
	ADC1->CR2|=0x40000000;		//start conversion
	while(!(ADC1->SR & 2)){}	//wait for conversion complete
	result=ADC1->DR;			//read conversion result
	ADC1->CR2&=~1;				//bit 0, ADC on/off (1=on, 0=off)
	return result;
}


/*void pow_int(){ //simulate the power reader's signal
	RCC->AHBENR|=0x01<<1;
	GPIOA->MODER&=~(0x03<<18);
	GPIOA->MODER|=0x01<<18;			//PA9 output simulation the signal
}

void adc_A1_init(){	//ADC for lmt84
	RCC->AHBENR|=0x01<<0;	//turn GPIOA
	GPIOA->MODER|=0x03<<2;	//turn PA1 analog

	RCC->APB2ENR|=0x01<<9;	//turn ADC
	ADC1->CR2&=~0x01<<1;	//single mode
	ADC1->SMPR3|=0x07<<3;	//384 cycle for channel 1
	ADC1->CR1&=~0x03<<24;	//resolution 12 bit
}

int read_A1()	//ADC for lmt84
{
	int result=0;
	int tem;
	ADC1->SQR5=1;				//conversion sequence to ch1
	ADC1->CR2|=1;				//bit 0, ADC on/off (1=on, 0=off)
	ADC1->CR2|=0x40000000;		//start conversion
	while(!(ADC1->SR & 2)){}	//wait for conversion complete
	result=ADC1->DR;			//read conversion result
	tem=10*1000*(1026-3300*result/4095)/5617; //follow the equation from datasheet lmt84
	ADC1->CR2&=~1;				//bit 0, ADC on/off (1=on, 0=off)
	return tem;
}

void adc_B0_init(){			//ADC for light sensor NSL-19M51
	RCC->AHBENR|=0x01<<1;	//turn GPIOB
	GPIOA->MODER|=0x03<<0;	//turn PB0 analog
	RCC->APB2ENR|=0x01<<9;	//turn ADC
	ADC1->CR2&=~0x01<<1;	//single mode
	ADC1->SMPR3|=0x07<<24;	//384 cycle for channel 1
	ADC1->CR1&=~0x03<<24;	//resolution 12 bit
}

uint16_t read_B0()	//function light, 0 dark, 1 soft light, 2 strong light
{
	int result=0;
	int tem;
	ADC1->SQR5=0x08;				//conversion sequence to ch8
	ADC1->CR2|=1;				//bit 0, ADC on/off (1=on, 0=off)
	ADC1->CR2|=0x40000000;		//start conversion
	while(!(ADC1->SR & 2)){}	//wait for conversion complete
	result=ADC1->DR;			//read conversion result
	if(result<2730){
		tem=1;
	}else if(result>3723){
		tem=3;
	}else{
		tem=2;
	}
	ADC1->CR2&=~1;				//bit 0, ADC on/off (1=on, 0=off)
	return tem;
}
	*/

void USART2_Init(void)
{
	RCC->APB1ENR|=0x00020000; 	//set bit 17 (USART2 EN)
	RCC->AHBENR|=0x00000001; 	//enable GPIOA port clock bit 0 (GPIOA EN)
	GPIOA->AFR[0]=0x00000700;	//GPIOx_AFRL p.188,AF7 p.177
	GPIOA->AFR[0]|=0x00007000;	//GPIOx_AFRL p.188,AF7 p.177
	GPIOA->MODER|=0x00000020; 	//MODER2=PA2(TX) to mode 10=alternate function mode. p184
	GPIOA->MODER|=0x00000080; 	//MODER2=PA3(RX) to mode 10=alternate function mode. p184

	USART2->BRR = 0x00000D05;	//9600 BAUD and crystal 32MHz. p710, D05
	USART2->CR1 = 0x00000008;	//TE bit. p739-740. Enable transmit
	USART2->CR1 |= 0x00000004;	//RE bit. p739-740. Enable receiver
	USART2->CR1 |= 0x00002000;	//UE bit. p739-740. Uart enable
}

void USART2_write(char data)
{
	//wait while TX buffer is empty
	while(!(USART2->SR&0x0080)){} 	//6. p736-737
		USART2->DR=(data);		//p739
}

void USART2_bufWrite(char *buf)
{
    while(*buf){
        USART2_write(*buf++);
    }
}

void USART2_intWrite(int t)
{
	char buf[100];
	uint8_t len=0;
	sprintf(buf,"%d",t);
	for(int i=0;buf[i]!='\0';i++){
		len++;
	}
	for(int i=0;i<len;i++){
		USART2_write(buf[i]);
	}
}

char USART2_read(){
	char data=0;
	while(!(USART2->SR&(0x02<<4))){}
	data=USART2->DR;
	return data;
}

/*static void bubbleShort5(int a[5]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
            if (a[j] > a[j+1]) {
                int tmp = a[j];
                a[j] = a[j+1];
                a[j+1] = tmp;
            }
        }
    }
}

int abs(int a){
	if(a>0){
		return a;
	}else{
		return 0-a;
	}
}
*/
void delay_Ms(int delay)
{
	int i=0;
	for(; delay>0;delay--)
		for(i=0;i<2460;i++); //measured with oscilloscope
}