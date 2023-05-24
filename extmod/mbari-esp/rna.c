/*
 * rna.c
 *
 *  Created on: Aug 11, 2022
 *      Author: sjensen
 */

#include "rna.h"
#include "stepISR.h"

int rnaHelp(char *optionline);

#define RNA_COUNTSperMICROL 16
#define RNA_PACERATE 2000000

int rnaInit(void)
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

		TIM5CC2set(1600,4000,0,1);  //set default for motor

	return 1;
}

signed char rna(int distance, unsigned int state)
{
	motionstruct rnaMoveState;

	if (distance < 0)
	{
		distance=-distance;
		GPIOC->BSRR = GPIO_BSRR_BR3; //clear GPIOC output for reverse
	}
	else
	{
		GPIOC->BSRR = GPIO_BSRR_BS3; //set GPIOC output for forward
	}

	TIM5CC2InitMove(distance, state);  //kick off the move

	do
	{
		if(getBreak())
		{
			TIM5CC2stop();
			return -1;
		}
		TIM5CC2status(&rnaMoveState);
	}while(rnaMoveState.running==1);

	if (rnaMoveState.running <0 )
    {
    	printf("Error\n\r");
    	fflush(stdout);
    }

    return 1;
}

int rnaDeliver(char *optionline)
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
		counts = volume * RNA_COUNTSperMICROL;
		pace = RNA_PACERATE/rate;
		TIM5CC2set(1600,pace,0,1);  //set pace to pump
		rna(counts, rna_NOSTATE);  //kick off the move
	}
	else
	{
		printf("microL and Rate required\n\r");
		fflush(stdout);
	}
	return 1;
}

int rnaMove(char *optionline)
{
    int distance;
    int result;

	result=sscanf(optionline,"%*s %*s %d",&distance);
	if (result==1)
	{
		if (distance == 0) return -1;
		rna(distance, rna_NOSTATE);  //kick off the move
	}
    return 1;
}


int rnaSet(char *optionline)
{
	int result;
	int acceleration;
	int plateau;
	int drift;
	int driftPace;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&acceleration, &plateau, &drift, &driftPace);
	if (result==4)
	{
		TIM5CC2set(acceleration,plateau,drift,driftPace);
	}

	return 1;
}

int rnaGet(char *optionline)
{
	motionstruct getstatus;
	TIM5CC2status(&getstatus);
	printf("Astep = %d\n\r",getstatus.asteps);
	printf("Plateau = %d\n\r",getstatus.pace);
	printf("Drift = %d\n\r",getstatus.drift);
	printf("Drift Pace = %d\n\r",getstatus.driftPace);
	fflush(stdout);
	return 1;
}

int rnaOptionNotFound(char *cmdline)
{
    printf("Unrecognized rnation\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef rnastruct;

rnastruct rnaoptions[]=
{
//    {"position",sizeof("position")-1,rnaPosition,""},
    {"deliver",sizeof("deliver")-1,rnaDeliver,"microL Rate"},
    {"move",sizeof("move")-1,rnaMove,"Distance Pace"},
    {"set",sizeof("set")-1,rnaSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,rnaGet,""},
    {"help",sizeof("help")-1,rnaHelp,""},
    {"",0,rnaOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(rnaoptions)/sizeof(rnastruct))-1)

int rnaHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",rnaoptions[optionindex].option,rnaoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int rnacommand(char *optionline)
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
				if(optlen==rnaoptions[optindex].optionlen)
				{
					if(memcmp(opt,rnaoptions[optindex].option,optlen)==0)
						{
						rnaoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) rnaoptions[optindex].optionfunc(optionline);
		}
//		else rnaPosition("rna position");
	return 0;
}

#undef OPTIONNUM

