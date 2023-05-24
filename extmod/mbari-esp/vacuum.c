/*
 * vacuum.c
 *
 *  Created on: Sep 16, 2022
 *      Author: sjensen
 */

#include "vacuum.h"

int vacuumOn(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (GPIO_BSRR_BR2); //clear GPIOD output
	return 1;
}

int vacuumOff(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (GPIO_BSRR_BS2); //set GPIOD output
	return 1;
}

int vacuumInit(void)
{
	//Configfure GPIOD2 as output to be used as direction
		RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIODEN; //enable clock to gpioc;
		GPIOD->MODER	&= ~(GPIO_MODER_MODE2);  //clear out mode
		GPIOD->MODER	|=	(GPIO_MODER_MODE2_0); //set as output
		GPIOD->OTYPER &=~(GPIO_OTYPER_OT2);  //set as push-pull
		GPIOD->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2); //set as low speed
		GPIOD->BSRR = (GPIO_BSRR_BS2); //set GPIOD output

		return 1;
}


int vacuumoptionNotFound(char *cmdline)
{
    printf("Unrecognized vacuum option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
}typedef vacuumstruct;

int vacuumHelp(char *optionline);

vacuumstruct vacuumoptions[]=
{
    {"on",sizeof("on")-1,vacuumOn},
    {"off",sizeof("off")-1,vacuumOff},
    {"help",sizeof("help")-1,vacuumHelp},
    {"",0,vacuumoptionNotFound}
};

#define vacuumNUM ((sizeof(vacuumoptions)/sizeof(vacuumstruct))-1)

int vacuumHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<vacuumNUM;optionindex++)
	{
		printf("%s\r\n",vacuumoptions[optionindex].option);
		fflush(stdout);
	}
    return 1;
}

int vacuumcommand(char *vacuumline)
{
	#define OPTIONLINE_LENGTH 256

	char opt[OPTIONLINE_LENGTH];
	int optlen;
	int result;
	int optindex;

		result=sscanf(vacuumline,"%*s %s",opt);
		if (result==1)
		{
			optlen=strlen(opt);
			for(optindex=0;optindex<vacuumNUM;optindex++)
			{
				if(optlen==vacuumoptions[optindex].optionlen)
				{
					if(memcmp(opt,vacuumoptions[optindex].option,optlen)==0)
						{
							vacuumoptions[optindex].optionfunc(vacuumline);
							break;
						}
				}
			}
			if(optindex==vacuumNUM) vacuumoptions[optindex].optionfunc(vacuumline);
		}
//		else vacuumPosition("vacuum position");
	return 0;
}

