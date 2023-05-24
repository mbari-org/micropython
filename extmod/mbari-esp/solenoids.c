/*
 * solenoids.c
 *
 *  Created on: Aug 11, 2022
 *      Author: sjensen
 */

#include "solenoids.h"

int solenoidInit(void)
{
	//Configfure GPIOD4-9 as output to be used as direction
		RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIODEN; //enable clock to gpioc;
		GPIOD->MODER	&= ~(GPIO_MODER_MODE4|GPIO_MODER_MODE5|GPIO_MODER_MODE6|GPIO_MODER_MODE7|GPIO_MODER_MODE8|GPIO_MODER_MODE9);  //clear out mode
		GPIOD->MODER	|=	(GPIO_MODER_MODE4_0|GPIO_MODER_MODE5_0|GPIO_MODER_MODE6_0|GPIO_MODER_MODE7_0|GPIO_MODER_MODE8_0|GPIO_MODER_MODE9_0); //set as output
		GPIOD->OTYPER &=~(GPIO_OTYPER_OT4|GPIO_OTYPER_OT5|GPIO_OTYPER_OT6|GPIO_OTYPER_OT7|GPIO_OTYPER_OT8|GPIO_OTYPER_OT9);  //set as push-pull
		GPIOD->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED4|GPIO_OSPEEDR_OSPEED5|GPIO_OSPEEDR_OSPEED6|GPIO_OSPEEDR_OSPEED7|GPIO_OSPEEDR_OSPEED8|GPIO_OSPEEDR_OSPEED9); //set as low speed
		GPIOD->BSRR = (GPIO_BSRR_BR4|GPIO_BSRR_BR5|GPIO_BSRR_BR6|GPIO_BSRR_BR7|GPIO_BSRR_BR8|GPIO_BSRR_BR9); //clear GPIOC output
		return 1;
}

int solenoidOn(char *optionline)
{
	unsigned int valve;
	int result;
	result=sscanf(optionline,"%*s %*s %u",&valve);
	if (result==1)
	{
		if((valve<1)||(valve>6)) return -1;
		GPIOD->BSRR = (0x1<<((valve-1)+4)); //set the set bits
	}

	return 1;
}

int solenoidOff(char *optionline)
{
	int result;
	unsigned int valve;
	result=sscanf(optionline,"%*s %*s %u",&valve);
	if (result==1)
	{
		if((valve<1)||(valve>6)) return -1;
		GPIOD->BSRR = (0x1<<((valve-1)+4+16)); //set the set bits
	}
	return 1;
}

int solenoidPosition(char *optionline)  //does nothing
{
	return 1;
}

int solenoidOptionNotFound(char *cmdline)
{
    printf("Unrecognized solenoid option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef solenoidstruct;

int solenoidHelp(char *optionline);

solenoidstruct solenoidoptions[]=
{
    {"on",sizeof("on")-1,solenoidOn,"valve"},
    {"off",sizeof("off")-1,solenoidOff,"valve"},
    {"position",sizeof("position")-1,solenoidPosition,""},
    {"help",sizeof("help")-1,solenoidHelp,""},
    {"",0,solenoidOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(solenoidoptions)/sizeof(solenoidstruct))-1)

int solenoidHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",solenoidoptions[optionindex].option,solenoidoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int solenoidcommand(char *optionline)
{
	#define OPTIONLINE_LENGTH 256

	char opt[OPTIONLINE_LENGTH];
	int optlen;
	int result;
	int optindex;

		result=sscanf(optionline,"%*s %s",opt);
		if (result==1)
		{
			optlen=strlen(opt);
			for(optindex=0;optindex<OPTIONNUM;optindex++)
			{
				if(optlen==solenoidoptions[optindex].optionlen)
				{
					if(memcmp(opt,solenoidoptions[optindex].option,optlen)==0)
						{
						solenoidoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) solenoidoptions[optindex].optionfunc(optionline);
		}
		else solenoidPosition("solenoid position");
	return 0;
}

