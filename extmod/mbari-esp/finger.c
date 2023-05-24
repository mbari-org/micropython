/*
 * finger.c
 *
 *  Created on: Aug 11, 2022
 *      Author: sjensen
 */


#include "finger.h"

int fingerUp(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (0x3<<16); //reset the set bits
	GPIOD->BSRR = (0x1); //raise finger
	return 1;
}

int fingerDown(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (0x3<<16); //reset the set bits
	GPIOD->BSRR = (0x2); //lower finger
	return 1;
}

int fingerSleep(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (0x3<<16); //reset the set bits
	return 1;
}

int fingerBrake(char *cmdline)
{
	char arguments[256];
	int result;
	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	GPIOD->BSRR = (0x3); //set the set bits
	return 1;
}

int fingerInit(void)
{
	//Configfure GPIOD0-1 as output to be used as direction
		RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIODEN; //enable clock to gpioc;
		GPIOD->MODER	&= ~(GPIO_MODER_MODE0|GPIO_MODER_MODE1);  //clear out mode
		GPIOD->MODER	|=	(GPIO_MODER_MODE0_0|GPIO_MODER_MODE1_0); //set as output
		GPIOD->OTYPER &=~(GPIO_OTYPER_OT0|GPIO_OTYPER_OT1);  //set as push-pull
		GPIOD->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0|GPIO_OSPEEDR_OSPEED1); //set as low speed
		GPIOD->BSRR = (GPIO_BSRR_BR0|GPIO_BSRR_BR1); //clear GPIOD output

//configure GPIOE8-11 as input
		RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOEEN; //enable clock to gpioc;
		GPIOE->MODER	&= ~(GPIO_MODER_MODE12|GPIO_MODER_MODE13);  //clear out mode to make input

		return 1;
}

int readFingerState(char *cmdline)
{
	unsigned int state;
	state = (GPIOE->IDR&(0x3<<12))>>12;
	return (int)state;
}

int fingerPosition(char *cmdline)
{
	unsigned int state;
	int result;
	char arguments[256];

	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
		fflush(stdout);
	}
	state = (GPIOE->IDR&(0x3<<12))>>12;
	printf("Finger state = %x (hex)\n\r",state);
	fflush(stdout);
	return 1;
}

int fingeroptionNotFound(char *cmdline)
{
    printf("Unrecognized Finger option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
}typedef fingerstruct;

int fingerHelp(char *optionline);

fingerstruct fingeroptions[]=
{
    {"up",sizeof("up")-1,fingerUp},
    {"down",sizeof("down")-1,fingerDown},
    {"sleep",sizeof("sleep")-1,fingerSleep},
    {"brake",sizeof("brake")-1,fingerBrake},
    {"position",sizeof("position")-1,fingerPosition},
    {"help",sizeof("help")-1,fingerHelp},
    {"",0,fingeroptionNotFound}
};

#define FINGERNUM ((sizeof(fingeroptions)/sizeof(fingerstruct))-1)

int fingerHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<FINGERNUM;optionindex++)
	{
		printf("%s\r\n",fingeroptions[optionindex].option);
		fflush(stdout);
	}
    return 1;
}

int fingercommand(char *fingerline)
{
	#define OPTIONLINE_LENGTH 256

	char opt[OPTIONLINE_LENGTH];
	int optlen;
	int result;
	int optindex;

		result=sscanf(fingerline,"%*s %s",opt);
		if (result==1)
		{
			optlen=strlen(opt);
			for(optindex=0;optindex<FINGERNUM;optindex++)
			{
				if(optlen==fingeroptions[optindex].optionlen)
				{
					if(memcmp(opt,fingeroptions[optindex].option,optlen)==0)
						{
							fingeroptions[optindex].optionfunc(fingerline);
							break;
						}
				}
			}
			if(optindex==FINGERNUM) fingeroptions[optindex].optionfunc(fingerline);
		}
		else fingerPosition("finger position");
	return 0;
}
