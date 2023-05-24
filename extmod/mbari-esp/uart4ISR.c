/*
 * uart4ISR.c
 *
 *  Created on: Sep 16, 2022
 *      Author: sjensen
 */

#include "uart4ISR.h"
#include "main.h"

static volatile int rx4front=0;
static volatile int rx4back=0;
static volatile int n4rx=0;
static volatile int tx4front=0;
static volatile int tx4back=0;
static volatile int n4tx=0;
static unsigned char rx4buf[uartrx4bufsize];
static unsigned char tx4buf[uarttx4bufsize];
static unsigned char ur4byte;


void UART4_IRQHandler(void)
{
 int flag=0;
 if (UART4->ISR&USART_ISR_RXNE) //was there something received
 {
    if (UART4->CR1&USART_CR1_RXNEIE) // is receive interrupt enabled?
    {//UART Rx interrupt
        ur4byte=(unsigned char)UART4->RDR; //pull out the byte which clears the interrupt
        if (n4rx<uartrx4bufsize)//can we accept this character
        {
            rx4buf[rx4front]=ur4byte;
            rx4front++;
			if(rx4front>=uartrx4bufsize) rx4front=0;
            n4rx+=1;
        }
    }
 }

 if ((UART4->CR1&USART_CR1_TXEIE)&&(UART4->ISR&USART_ISR_TXE))
    {//UART Tx interrupt
        if (n4tx>0)
        {
            UART4->TDR=tx4buf[tx4back];
            tx4back++;
            if(tx4back>=uarttx4bufsize)tx4back=0;;
            n4tx-=1;
        }
        if (n4tx==0) UART4->CR1&=~(USART_CR1_TXEIE); //turn off interrupts
    }
return;
}

int uart4Init(void)
{
// ************   Trying to get the LPUART up and running ************************
	//this should select the LPUART as the alternate function of pin PG7 TX and PG8 RX
#define UART_AF 0x8
//	PWR->CR2 |= PWR_CR2_IOSV;  //since LPUART can run while asleep this helps deal with that, is it needed?
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOAEN; //enable GPIOA on AHB2
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0_1|GPIO_OSPEEDR_OSPEED0_0); //set output speed to very fast
 	GPIOA->MODER	|=	GPIO_MODER_MODE0_1;  //set the mode of PG7 to 0x2 which is alternate function.
	GPIOA->MODER	&= ~GPIO_MODER_MODE0_0;
	GPIOA->AFR[0] |= (UART_AF<<4);   //set the alternate function as LPUART TX

 	GPIOA->MODER	|=	GPIO_MODER_MODE1_1;  //set the mode of PG8 to 0x2 which is alternate function.
	GPIOA->MODER	&= ~GPIO_MODER_MODE1_0;
	GPIOA->AFR[0] |= (UART_AF<<0);   //set the alternate function as LPUART RX

//	RCC->CCIPR |= RCC_CCIPR_UART4SEL_1;  //this should set it to 0x2 which is HSI16  leaving at 00 = PCLK it works
	RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;  //Enable the clock to the LPUART

	UART4->CR3 |= USART_CR3_OVRDIS; //disblae overun error interrupts
	UART4->BRR = (278);    ///this seems to set it at 115200
	UART4->CR1 |= (USART_CR1_UE|USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE);  //USART Enable. May need to be done last

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // UART receive interrupts should be high priority.
    uint32_t uart_pri_encoding = NVIC_EncodePriority( 0, 2, 0 );
    NVIC_SetPriority( UART4_IRQn, uart_pri_encoding );
    NVIC_EnableIRQ( UART4_IRQn );

    return(1);
}

int u4txaddbyte(unsigned char dbyte)
{
    while(n4tx>=uarttx4bufsize); //wait for room in buffer
//    if (n4tx<uarttx4bufsize) //can it accept another
//    {
    	UART4->CR1&=~(USART_CR1_TXEIE);  //Disable transmit interrupt while adding character
        tx4buf[tx4front]=dbyte;
        tx4front++;
        if(tx4front>=uarttx4bufsize) tx4front=0;
        n4tx+=1;
        UART4->CR1|=(USART_CR1_TXEIE);  //enable tx interrupts
        return n4tx; //how many bytes in buffer
//    }
//    else
//        return (signed char) -1; //buffer full
}

int u4rxaddbyte(unsigned char dbyte)
{
    if (n4rx<uartrx4bufsize) //can it accept another
    {
    	UART4->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while loading character
        rx4buf[rx4front]=dbyte;
        rx4front++;
		if(rx4front>=uartrx4bufsize) rx4front=0;
        n4rx+=1;
        UART4->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
        return n4rx; //how many bytes in buffer
    }
    else
        return -1; //buffer full
}

int u4rxgetbyte(unsigned char *dbyte)
{

    if (n4rx>0)  //is there any data to return
    {
    	UART4->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while loading character
        *dbyte=rx4buf[rx4back];
        rx4back++;
		if(rx4back>=uartrx4bufsize) rx4back=0;
        n4rx-=1;
        UART4->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
        return n4rx;   //how many left
    }
    else return -1;  //no data to return
}


int nin4rxbuf(void)
{
    return n4rx;
}

int u4txroom(void)
{
    return(uarttx4bufsize-n4tx-1);
}

int uart4rxflush(void)
{
	UART4->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while loading character
    rx4front=0;
    rx4back=0;
    n4rx=0;
    UART4->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
	return 1;
}

