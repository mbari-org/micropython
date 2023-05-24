/*
 * flush.c
 *
 *  Created on: Sep 13, 2022
 *      Author: sjensen
 */

#include "flush.h"
#include "stepISR.h"

#define FLUSH_COUNTSperMICROL 16
#define FLUSH_PACERATE 2000000

int flushHelp(char *optionline);

int flushInit(void)
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

		TIM5CC3set(1600,4000,0,1);  //set default for motor

	return 1;
}

signed char flush(int distance, unsigned int state)
{
	motionstruct flushMoveState;

	if (distance < 0)
	{
		distance=-distance;
		GPIOC->BSRR = GPIO_BSRR_BR4; //clear GPIOC output for reverse
	}
	else
	{
		GPIOC->BSRR = GPIO_BSRR_BS4; //set GPIOC output for forward
	}

	TIM5CC3InitMove(distance, state);  //kick off the move

	do
	{
		if(getBreak())
		{
			TIM5CC3stop();
			return -1;
		}
		TIM5CC3status(&flushMoveState);
	}while(flushMoveState.running==1);

	if (flushMoveState.running <0 )
    {
    	printf("Error\n\r");
    	fflush(stdout);
    }

    return 1;
}

int flushMove(char *optionline)
{
    int distance;
    int result;

	result=sscanf(optionline,"%*s %*s %d",&distance);
	if (result==1)
	{
		if (distance == 0) return -1;
		flush(distance, flush_NOSTATE);  //kick off the move
	}
    return 1;
}

int flushDeliver(char *optionline)
{
    int volume;
    int rate;
    int counts;
    int pace;
    int result;

	result=sscanf(optionline,"%*s %*s %d %d",&volume, &rate);
	if (result==2)
	{
		if (volume == 0) return -1;
		if (rate <= 0) return -2;
		counts = volume * FLUSH_COUNTSperMICROL;
		pace = FLUSH_PACERATE/rate;
		TIM5CC3set(1600,pace,0,1);  //set pace to pump
		flush(counts, flush_NOSTATE);  //kick off the move
	}
	else
	{
		printf("microL and Rate required\n\r");
		fflush(stdout);
	}
	return 1;
}

int flushSet(char *optionline)
{
	int result;
	int acceleration;
	int plateau;
	int drift;
	int driftPace;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&acceleration, &plateau, &drift, &driftPace);
	if (result==4)
	{
		TIM5CC3set(acceleration,plateau,drift,driftPace);
	}

	return 1;
}

int flushGet(char *optionline)
{
	motionstruct getstatus;
	TIM5CC3status(&getstatus);
	printf("Astep = %d\n\r",getstatus.asteps);
	printf("Plateau = %d\n\r",getstatus.pace);
	printf("Drift = %d\n\r",getstatus.drift);
	printf("Drift Pace = %d\n\r",getstatus.driftPace);
	fflush(stdout);
	return 1;
}

int flushOptionNotFound(char *cmdline)
{
    printf("Unrecognized flushtion\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef flushstruct;

flushstruct flushoptions[]=
{
    {"deliver",sizeof("deliver")-1,flushDeliver,"microL Rate"},
    {"move",sizeof("move")-1,flushMove,"Distance Pace"},
    {"set",sizeof("set")-1,flushSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,flushGet,""},
    {"help",sizeof("help")-1,flushHelp,""},
    {"",0,flushOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(flushoptions)/sizeof(flushstruct))-1)

int flushHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",flushoptions[optionindex].option,flushoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int flushcommand(char *optionline)
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
				if(optlen==flushoptions[optindex].optionlen)
				{
					if(memcmp(opt,flushoptions[optindex].option,optlen)==0)
						{
						flushoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) flushoptions[optindex].optionfunc(optionline);
		}
//		else flushPosition("flush position");
	return 0;
}

#undef OPTIONNUM



