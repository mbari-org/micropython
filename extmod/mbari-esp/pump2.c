/*
 * pump2.c
 *
 *  Created on: Aug 15, 2022
 *      Author: sjensen
 */

#include "slide.h"

int pump2Init(void)
{

//Configfure GPIOC4 as output to be used as direction
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOCEN; //enable clock to gpioc;
	GPIOC->MODER	&= ~GPIO_MODER_MODE4;  //clear out mode
	GPIOC->MODER	|=	GPIO_MODER_MODE4_0; //set as output
	GPIOC->OTYPER &=~GPIO_OTYPER_OT4;  //set as push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED4; //set as low speed
	GPIOC->BSRR = GPIO_BSRR_BR4; //clear GPIOC output

//Configure Timer5 Capture 3 on PF8
	TIM5CC3Initialize();

	return 1;
}
