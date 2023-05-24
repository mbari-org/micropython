/*
 * clamp.c
 *
 *  Created on: Aug 15, 2022
 *      Author: sjensen
 */

#include "clamp.h"
#include "stepISR.h"

int clampInit(void)
{

//Configfure GPIOC2 as output to be used as direction
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOCEN; //enable clock to gpioc;
	GPIOC->MODER	&= ~GPIO_MODER_MODE2;  //clear out mode
	GPIOC->MODER	|=	GPIO_MODER_MODE2_0; //set as output
	GPIOC->OTYPER &=~GPIO_OTYPER_OT2;  //set as push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED2; //set as low speed
	GPIOC->BSRR = GPIO_BSRR_BR2; //clear GPIOC output

//Configure Timer2 Capture 4 on PB11
	TIM2CC4Initialize();
	TIM2CC4set(26400,2000,0,32768);

//configure GPIOE8-11 as input
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOEEN; //enable clock to gpioc;
	GPIOE->MODER	&= ~(GPIO_MODER_MODE0|GPIO_MODER_MODE1|GPIO_MODER_MODE14|GPIO_MODER_MODE15);  //clear out mode to make input


	return 1;
}

int clampHelp(char *optionline);

int readclampState(void)
{
	unsigned int state;
	state=GPIOE->IDR;
	state = ((state&(0x3<<14))>>12)|(state&0x3);
	return state;
}

int clampPosition(char *cmdline)
{
	unsigned int state;
	int result;
	char arguments[256];

	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
	}
	state=GPIOE->IDR;
	state = ((state&(0x3<<14))>>12)|(state&0x3);
	printf("clamp state = %x (hex)\n\r",state);
/*
	for(result=0;result<NUMTUBES;result++)
	{
		if(tubes[result].up==state)
		{
			printf("Cylinder at Tube %d\n\r",tubes[result].number);
		}
	}
	*/
	fflush(stdout);

	return 1;
}

signed char clamp(int distance, unsigned int state)
{
	motionstruct clampMoveState;

	if (distance < 0)
	{
		distance=-distance;
		GPIOC->BSRR = GPIO_BSRR_BR2; //clear GPIOC output for reverse
	}
	else
	{
		GPIOC->BSRR = GPIO_BSRR_BS2; //set GPIOC output for forward
	}

	TIM2CC4InitMove(distance, state);  //kick off the move

	do
	{
		if(getBreak())
		{
			TIM2CC4stop();
			return -1;
		}
		TIM2CC4status(&clampMoveState);
	}while(clampMoveState.running==1);

	if (clampMoveState.running <0 )
    {
    	printf("Error\n\r");
    	fflush(stdout);
    }

    return 1;
}

int clampMove(char *optionline)
{
    int distance;
    int result;

	result=sscanf(optionline,"%*s %*s %d",&distance);
	if (result==1)
	{
		if (distance == 0) return -1;
		clamp(distance, clamp_NOSTATE);  //kick off the move
	}
    return 1;
}

int clampToState(char *optionline)
{
	int result;
	int newState;
	int distance;
	result=sscanf(optionline,"%*s %*s %d %d",&newState, &distance);

	if (result==2)
	{
		clamp(distance,newState);
		return 1;
	}
	printf("Required State and Distance\n\r");
	fflush(stdout);
	return -1;
}


int clampSet(char *optionline)
{
	int result;
	int acceleration;
	int plateau;
	int drift;
	int driftPace;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&acceleration, &plateau, &drift, &driftPace);
	if (result==4)
	{
		TIM2CC4set(acceleration,plateau,drift,driftPace);
	}

	return 1;
}

int clampGet(char *optionline)
{
	motionstruct getstatus;
	TIM2CC4status(&getstatus);
	printf("Astep = %d\n\r",getstatus.asteps);
	printf("Plateau = %d\n\r",getstatus.pace);
	printf("Drift = %d\n\r",getstatus.drift);
	printf("Drift Pace = %d\n\r",getstatus.driftPace);
	fflush(stdout);
	return 1;
}

int clampOptionNotFound(char *cmdline)
{
    printf("Unrecognized clamp option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef clampstruct;

clampstruct clampoptions[]=
{
    {"position",sizeof("position")-1,clampPosition,""},
    {"move",sizeof("move")-1,clampMove,"Distance Pace"},
    {"state",sizeof("state")-1,clampToState,"State Distance"},
    {"set",sizeof("set")-1,clampSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,clampGet,""},
    {"help",sizeof("help")-1,clampHelp,""},
    {"",0,clampOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(clampoptions)/sizeof(clampstruct))-1)

int clampHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",clampoptions[optionindex].option,clampoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int clampcommand(char *optionline)
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
				if(optlen==clampoptions[optindex].optionlen)
				{
					if(memcmp(opt,clampoptions[optindex].option,optlen)==0)
						{
						clampoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) clampoptions[optindex].optionfunc(optionline);
		}
//		else clampPosition("clamp position");
	return 0;
}

#undef OPTIONNUM

