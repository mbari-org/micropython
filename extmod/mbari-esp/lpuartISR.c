/*
 * lpuartInit.c
 *
 *  Created on: Aug 9, 2022
 *      Author: sjensen
 */

//#include <core_armv81mml.h>

#include "lpuartISR.h"
#include "main.h"

static volatile int rxfront=0;
static volatile int rxback=0;
static volatile int nrx=0;
static volatile int txfront=0;
static volatile int txback=0;
static volatile int ntx=0;
static unsigned char rxbuf[uartrxbufsize];
static unsigned char txbuf[uarttxbufsize];
static unsigned char urbyte;
//static unsigned char utbyte;
static volatile int uartEOL=0;
//static unsigned int fault=0;

static volatile int intBreak=0;

int setBreak(void)
{
	intBreak = 1;
	return intBreak;
}

int getBreak(void)
{
	return intBreak;
}

int clrBreak(void)
{
	intBreak = 0;
	return intBreak;
}

void LPUART1_IRQHandler(void)
{
 int flag=0;
 if (LPUART1->ISR&USART_ISR_RXNE) //was there something received
 {
	 flag=1;
    if (LPUART1->CR1&USART_CR1_RXNEIE) // is receive interrupt enabled?
    {//UART Rx interrupt
        urbyte=(unsigned char)LPUART1->RDR; //pull out the byte which clears the interrupt
        if(urbyte==0x03) intBreak=1;
        else if((urbyte==0x08)&&(nrx>0)&&(ntx<(uarttxbufsize-3)))
        {
            rxfront--;
    		if(rxfront<0) rxfront=(uartrxbufsize-1);
            nrx-=1;
            txbuf[txfront]=0x08;
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            txbuf[txfront]=' ';
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            txbuf[txfront]=0x08;
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            if (ntx==3)  LPUART1->CR1|=USART_CR1_TXEIE;  //this enables tx interrupts since there is a byte to send
        }
        else if(((urbyte==10)||(urbyte==13))&&(nrx<uartrxbufsize)&&(ntx<(uarttxbufsize-2)))
        {
            rxbuf[rxfront]='\n';
            rxfront++;
			if(rxfront>=uartrxbufsize) rxfront=0;
            nrx+=1;
            txbuf[txfront]=13;
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            txbuf[txfront]=10;
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            uartEOL= 1;
            if (ntx==2)  LPUART1->CR1|=USART_CR1_TXEIE;  //this enables tx interrupts since there is a byte to send
        }
        else if ((nrx<uartrxbufsize-1)&&(ntx<uarttxbufsize)) //can we accept this character and echo and leave room for \r
        {
            rxbuf[rxfront]=urbyte;
            rxfront++;
			if(rxfront>=uartrxbufsize) rxfront=0;
            nrx+=1;
            txbuf[txfront]=urbyte;
            txfront++;
            if(txfront>=uarttxbufsize) txfront=0;
            ntx+=1;
            if (ntx==1)  LPUART1->CR1|=USART_CR1_TXEIE;  //this enables tx interrupts since there is a byte to send
//            if ((urbyte == (unsigned char) 13)||(urbyte == (unsigned char) 10))
//            {
//                uartEOL= 1;
//            }
//            else if(nrx==uartrxbufsize) uartEOL=-1;
        }
        else
        	flag=4;
//        else uartEOL=-1;
    }
    }

    if ((LPUART1->CR1&USART_CR1_TXEIE)&&(LPUART1->ISR&USART_ISR_TXE))
    {//UART Tx interrupt
    	flag = 2;
        if (ntx>0)
        {
            LPUART1->TDR=txbuf[txback];
            txback++;
            if(txback>=uarttxbufsize)txback=0;;
            ntx-=1;
        }
        if (ntx==0) LPUART1->CR1&=~(USART_CR1_TXEIE); //turn off interrupts
    }
    if(flag == 0)
    	flag = 3;
return;
 }


int getline(char *string, int length)
{
	unsigned char next;
	int status;
	int len;
	int space;

	if (uartEOL<=0) return -1;
	space=0;
	len=0;

	while (len<(length-1))
	{
		status=urxgetbyte(&next);
		if(status<0)
			return -1;
		switch(next)
		{
		case '\n':
		case '\r':	*string=0;
					return len;
		case ' ':	if(len!=0)
					{
					if (space==0)
					{
						space = 1;
						*string++=next;
						len++;
					}
					}
					break;
		default:	space=0;
					*string++=next;
					len++;
		}
	}
	return len;
}

int _write(int file, char *ptr, int len)
{
	int index;
	int error;

	for(index=0;index<len;index++)
	{
		error = utxaddbyte(*ptr++);
		if (error < 0) return error;
	}
	return len;
}

int _read(int file, char *ptr, int len)
{
	int index=0;
	int error=-1;
    int front=rxfront;
    int back=rxback;

	while(front!=back)
	{
		if((rxbuf[back]=='\n')||(rxbuf[back]=='\r'))
		{
			for (index = 0; index < len; index++,ptr++)
			{
				error = urxgetbyte((unsigned char *)ptr);
				if (error<0) return error;
				if(error==0) return ++index;
				if((*ptr =='\n')||(*ptr=='\r')) return ++index;
			}
		}
		back++;
		if (back>=uartrxbufsize) back=0;
	}

/*
	for (index = 0; index < len; index++,ptr++)
	{
		error = urxgetbyte((unsigned char *)ptr);
		if (error<0) break;
		if(error==0) return ++index;
	}
*/
return error;
}

int lpuartInit(void)
{
// ************   Trying to get the LPUART up and running ************************
	//this should select the LPUART as the alternate function of pin PG7 TX and PG8 RX
#define LPUART_AF 0x8
	PWR->CR2 |= PWR_CR2_IOSV;  //since LPUART can run while asleep this helps deal with that, is it needed?
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOGEN; //enable GPIOG on AHB2
	GPIOG->OSPEEDR |= (GPIO_OSPEEDR_OSPEED7_1|GPIO_OSPEEDR_OSPEED7_0); //set output speed to very fast
 	GPIOG->MODER	|=	GPIO_MODER_MODE7_1;  //set the mode of PG7 to 0x2 which is alternate function.
	GPIOG->MODER	&= ~GPIO_MODER_MODE7_0;
	GPIOG->AFR[0] |= (LPUART_AF<<28);   //set the alternate function as LPUART TX

 	GPIOG->MODER	|=	GPIO_MODER_MODE8_1;  //set the mode of PG8 to 0x2 which is alternate function.
	GPIOG->MODER	&= ~GPIO_MODER_MODE8_0;
	GPIOG->AFR[1] |= (LPUART_AF<<0);   //set the alternate function as LPUART RX

//	RCC->CCIPR |= RCC_CCIPR_LPUART1SEL_1;  //this should set it to 0x2 which is HSI16  leaving at 00 = PCLK it works
	RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;  //Enable the clock to the LPUART

	LPUART1->CR3 |= USART_CR3_OVRDIS; //disblae overun error interrupts
	LPUART1->BRR = (107510*8);    ///this seems to set it at 9600
	LPUART1->CR1 |= (USART_CR1_UE|USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE);  //USART Enable. May need to be done last

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // UART receive interrupts should be high priority.
    uint32_t uart_pri_encoding = NVIC_EncodePriority( 0, 2, 0 );
    NVIC_SetPriority( LPUART1_IRQn, uart_pri_encoding );
    NVIC_EnableIRQ( LPUART1_IRQn );

    return(1);
}

int utxaddbyte(unsigned char dbyte)
{
    while(ntx>=uarttxbufsize); //wait for room in buffer
//    if (ntx<uarttxbufsize) //can it accept another
//    {
    	LPUART1->CR1&=~(USART_CR1_TXEIE);  //Disable transmit interrupt while adding character
        txbuf[txfront]=dbyte;
        txfront++;
        if(txfront>=uarttxbufsize) txfront=0;
        ntx+=1;
        LPUART1->CR1|=(USART_CR1_TXEIE);  //enable tx interrupts
        return ntx; //how many bytes in buffer
//    }
//    else
//        return (signed char) -1; //buffer full
}

int urxaddbyte(unsigned char dbyte)
{
    if (nrx<uartrxbufsize) //can it accept another
    {
    	LPUART1->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while loading character
        rxbuf[rxfront]=dbyte;
        rxfront++;
		if(rxfront>=uartrxbufsize) rxfront=0;
        nrx+=1;
        LPUART1->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
        if ((dbyte==13)||(dbyte==10))
        {
            uartEOL=1;
        }
        return nrx; //how many bytes in buffer
    }
    else
        return -1; //buffer full
}

int urxgetbyte(unsigned char *dbyte)
{
	int front;
	int back;

    if (nrx>0)  //is there any data to return
    {
    	LPUART1->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while loading character
        *dbyte=rxbuf[rxback];
        rxback++;
		if(rxback>=uartrxbufsize) rxback=0;
        nrx-=1;
        LPUART1->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on

        /*
        if (uartEOL < 0)
            if (nrx<uartrxbufsize)
        {
            uartEOL=0;
        }
        */
        if ((*dbyte==13)||(*dbyte==10))
        {
        	LPUART1->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while clearing flag
            uartEOL=0;
            back=rxback;
            front=rxfront;
            LPUART1->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
            while (back!=front) //search for another return
            {
                if ((rxbuf[back]==13)||(rxbuf[back]==10))
                {
                	LPUART1->CR1&=~(USART_CR1_RXNEIE);  //disable interrupt while clearing flag
                    uartEOL=1;
                    LPUART1->CR1|=(USART_CR1_RXNEIE); //turn interrupts back on
                    return nrx;
                }
                if (++back>=uartrxbufsize) back=0;
            }
            return nrx;
        }
        return nrx;   //how many left
    }
    else return -1;  //no data to return
}

int uEOL(void)
{
    return uartEOL;
}

int ninrxbuf(void)
{
    return nrx;
}

int utxroom(void)
{
    return(uarttxbufsize-ntx-1);
}


