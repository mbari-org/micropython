/*
 * pump1.c
 *
 *  Created on: Aug 15, 2022
 *      Author: sjensen
 */

#include "slide.h"

int pump1Init(void)
{

//Configfure GPIOC3 as output to be used as direction
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOCEN; //enable clock to gpioc;
	GPIOC->MODER	&= ~GPIO_MODER_MODE3;  //clear out mode
	GPIOC->MODER	|=	GPIO_MODER_MODE3_0; //set as output
	GPIOC->OTYPER &=~GPIO_OTYPER_OT3;  //set as push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED3; //set as low speed
	GPIOC->BSRR = GPIO_BSRR_BR3; //clear GPIOC output

//Configure Timer5 Capture 2 on PF7
	TIM5CC2Initialize();

	return 1;
}
