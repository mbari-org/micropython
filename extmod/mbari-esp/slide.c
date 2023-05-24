/*
 * slide.c
 *
 *  Created on: Aug 11, 2022
 *      Author: sjensen
 */

#include "slide.h"
#include "stepISR.h"

#define SLIDEGLIDE 1000
#define SLIDEPOSITION ((GPIOE->IDR&(SLIDE_MASK<<8))>>8)

struct slidepositions
{
int at;
int	to;
int unfoundDistance;
int foundDistance;
int finalState;
int finalDistance;
} typedef slidepositiondef;

struct slidePosNames
{
	char *namestr;
	int namelen;
	int positionNumber;
	unsigned int state;
	slidepositiondef *array;
} typedef slidePosNamesdef;

#define POSNUM ((sizeof(slideNameToNumber)/sizeof(slidePosNamesdef)))

#define PICK 1
#define FILTER 2
#define DROP 3
#define FLUSH 4

/* these are for first 25mm
#define PICKSTATE 0x09
#define FILTERSTATE 0x0e
#define DROPSTATE 0x00
#define FLUSHSTATE 0x0f
*/

//these are for first 47mm
#define PICKSTATE 0x01
#define FILTERSTATE 0x0e
#define DROPSTATE 0x00
#define FLUSHSTATE 0x03

/*
slidepositiondef slidePos[] =
{
	{0,0x8,0x8,0x8},
	{1,0x9,0xe,0xf},
	{2,0xe,0x0,0x9},
	{4,0xf,0x9,0x0},
	{3,0x4,0xb,0xf},
	{5,0x0,0xe,0xf},
	{6,0xb,0xb,0xf},
	{7,0xf,0xe,0xf},
};
32541
1000
-31509
-31509
-114594
-225229
-254057
-254057

*/
//moving to pick from current state
slidepositiondef pickPos[] =
{
	{0x8,0xb,(32623+SLIDEGLIDE),(32623+SLIDEGLIDE),0x9,-1000},
	{PICKSTATE,0xb,(28381+SLIDEGLIDE),(1000+SLIDEGLIDE),0x9,-1000},
	{0xb,0x9,(-31509-SLIDEGLIDE),(-31509-SLIDEGLIDE),0,0},
	{FLUSHSTATE,0x9,(-114864-SLIDEGLIDE),(-32128-SLIDEGLIDE),0,0},
	{FILTERSTATE,0x9,(-142854-SLIDEGLIDE),(-114594-SLIDEGLIDE),0,0},
	{0xc,0x9,(-225949-SLIDEGLIDE),(-225229-SLIDEGLIDE),0,0},
	{0x4,0x9,(-254777-SLIDEGLIDE),(-254057-SLIDEGLIDE),0,0},
	{DROPSTATE,0x9,(-253669-SLIDEGLIDE),(-254057-SLIDEGLIDE),0,0},

//	{FILTER,0xe,(114846+SLIDEGLIDE),0,0},
//	{DROP,0x0,(253729+SLIDEGLIDE),0,0},
//	{FLUSH,0xf,(31137+SLIDEGLIDE),0,0},
};

//#define NUMSLIDEPOSITIONS ((sizeof(slidePos))/sizeof(slidepositiondef))

slidepositiondef flushPos[] =
{
		{0x8,0xf,(64050+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{PICKSTATE,0xf,(59889+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{0xb,0xf,(31509+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{FLUSHSTATE,0xb,(-83085-SLIDEGLIDE),(32623+SLIDEGLIDE),0xf,1000},
		{FILTERSTATE,0xb,(-111345-SLIDEGLIDE),(32623+SLIDEGLIDE),0xf,1000},
		{0xc,0xb,(-193720-SLIDEGLIDE),(32623+SLIDEGLIDE),0xf,1000},
		{0x4,0xb,(-222548-SLIDEGLIDE),(32623+SLIDEGLIDE),0xf,1000},
		{DROPSTATE,0xb,(-228985-SLIDEGLIDE),(32623+SLIDEGLIDE),0xf,1000}

//	{PICK,0x9,(-31149-SLIDEGLIDE),0,0},
//	{FILTER,0xe,(83732+SLIDEGLIDE),0,0},
//	{DROP,0x0,(222540+SLIDEGLIDE),0,0},
};

slidepositiondef filterPos[] =
{
		{0x8,0xe,(147135+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{PICKSTATE,0xe,(142974+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{0xb,0xe,(114594+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{FLUSHSTATE,0xe,(83085+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{FILTERSTATE,0xf,(-28260-SLIDEGLIDE),(32623+SLIDEGLIDE),0xe,1000},
		{0xc,0xf,(-110635-SLIDEGLIDE),(32623+SLIDEGLIDE),0xe,1000},
		{0x4,0xf,(-139463-SLIDEGLIDE),(32623+SLIDEGLIDE),0xe,1000},
		{DROPSTATE,0xf,(-145900-SLIDEGLIDE),(32623+SLIDEGLIDE),0xe,1000}

//		{PICKSTATE,0xe,(114846+SLIDEGLIDE),0,0},
//	{FLUSHSTATE,0xf,(83732+SLIDEGLIDE),0xe,1000},
//	{DROPSTATE,0xf,(-222756-SLIDEGLIDE),0xe,1000}

//	{PICK,0x9,(-114864-SLIDEGLIDE),0,0},
//	{FLUSH,0xb,(-83963-SLIDEGLIDE),0xf,1000},
//	{DROP,0x0,(138805+SLIDEGLIDE),0,0},
};

slidepositiondef dropPos[] =
{
		{0x8,0x0,(286598+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{PICKSTATE,0x0,(282437+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{0xb,0x0,(254057+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{FLUSHSTATE,0x0,(222548+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{FILTERSTATE,0x0,(139463+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{0xc,0x0,(111203+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{0x4,0x0,(28828+SLIDEGLIDE),(32623+SLIDEGLIDE),0,0},
		{DROPSTATE,0x4,(-6437-SLIDEGLIDE),(32623+SLIDEGLIDE),0x0,1000}

//	{FILTERSTATE,0x0,(138805+SLIDEGLIDE),0,0},
//	{FLUSHSTATE,0x0,(222540+SLIDEGLIDE),0,0},
//	{PICKSTATE,0x0,(253729+SLIDEGLIDE),0,0}

//	{FILTER,0xf,(-139004-SLIDEGLIDE),0xe,1000},
//	{FLUSH,0xb,(-222756-SLIDEGLIDE),0xf,1000},
//	{PICK,0x9,(-253669-SLIDEGLIDE),0,0},
};

slidePosNamesdef slideNameToNumber[] =
{
		{"pick",sizeof("pick")-1,1,0x9,pickPos},
		{"filter",sizeof("filter")-1,2,0xe,filterPos},
		{"drop",sizeof("drop")-1,3,0x0,dropPos},
		{"flush",sizeof("flush")-1,4,0xf,flushPos}
};

int slideHelp(char *optionline);

int slideInit(void)
{

//Configfure GPIOC1 as output to be used as direction
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOCEN; //enable clock to gpioc;
	GPIOC->MODER	&= ~GPIO_MODER_MODE1;  //clear out mode
	GPIOC->MODER	|=	GPIO_MODER_MODE1_0; //set as output
	GPIOC->OTYPER &=~GPIO_OTYPER_OT1;  //set as push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED1; //set as low speed
	GPIOC->BSRR = GPIO_BSRR_BR1; //clear GPIOC output

//Configure Timer2 Capture 3 on PB10
	TIM2CC3Initialize();

	TIM2CC3set(1000,2000,2*SLIDEGLIDE,16000);

//configure GPIOE8-11 as input
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOEEN; //enable clock to gpioe;
	GPIOE->MODER	&= ~(GPIO_MODER_MODE8|GPIO_MODER_MODE9|GPIO_MODER_MODE10|GPIO_MODER_MODE11);  //clear out mode to make input

	return 1;
}

int readSlideState(void)
{
	unsigned int state;
	state = (GPIOE->IDR&(0xf<<8))>>8;
	return state;
}

int slidePosition(char *cmdline)
{
	unsigned int state;
	int result;
	char arguments[256];

	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
	}
	state = SLIDEPOSITION;
	printf("Slide state = %x (hex)\n\r",state);
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

signed char slide(int distance, unsigned int state)
{
	motionstruct slideMoveState;

	if (distance < 0)
	{
		distance=-distance;
		GPIOC->BSRR = GPIO_BSRR_BR1; //clear GPIOC output for reverse
	}
	else
	{
		GPIOC->BSRR = GPIO_BSRR_BS1; //set GPIOC output for forward
	}

	TIM2CC3InitMove(distance, state);  //kick off the move

	do
	{
		if(getBreak())
		{
			TIM2CC3stop();
			return -1;
		}
		TIM2CC3status(&slideMoveState);
	}while(slideMoveState.running==1);

	if (slideMoveState.running <0 )
    {
    	printf("Error\n\r");
    	fflush(stdout);
    	return -1;
    }

    return 1;
}

int slideMove(char *optionline)
{
    int distance;
    int result;

	result=sscanf(optionline,"%*s %*s %d",&distance);
	if (result==1)
	{
		if (distance == 0) return -1;
		slide(distance, SLIDE_NOSTATE);  //kick off the move
	}
    return 1;
}

int slideToState(char *optionline)
{
	int result;
	int newState;
	int distance;
	result=sscanf(optionline,"%*s %*s %d %d",&newState, &distance);

	if (result==2)
	{
		slide(distance,newState);
		return 1;
	}
	printf("Required State and Distance\n\r");
	fflush(stdout);
	return -1;
}

int slideMoveToPosition(char *optionline)
{
	static int found=0;
	int result;
	int stateIndex;
	int newPosition,currentPosition;
	int positionState,positionDistance=0;
	int finalState,finalDistance=0;
	slidepositiondef *newPos=NULL;
	char newString[100];
	int posindex;
	int poslen;

//	result=sscanf(optionline,"%*s %*s %d",&newPosition);
	result=sscanf(optionline,"%*s %*s %s",newString);

	if (result==1)
	{
		poslen=strlen(newString);
		for(posindex=0;posindex<POSNUM;posindex++)
		{
			if(poslen==slideNameToNumber[posindex].namelen)
			{
				if(memcmp(newString,slideNameToNumber[posindex].namestr,poslen)==0)
					{
						newPos=slideNameToNumber[posindex].array;
						break;
					}
			}
		}
		if(posindex==POSNUM)
		{
			printf("Unknown slide position: %s\r\n",newString);
			fflush(stdout);
			return -1;
		}


		currentPosition=SLIDEPOSITION;

		for(stateIndex=0;stateIndex<8;stateIndex++)
		{
			if(currentPosition==newPos[stateIndex].at)
			{
				printf("%d,%d,%d\r\n",found,newPos[stateIndex].foundDistance,newPos[stateIndex].unfoundDistance);
				if(found==0)
				{
					if(slide(newPos[stateIndex].unfoundDistance,newPos[stateIndex].to)>0) found=1;
				}
				else
				{
					if(slide(newPos[stateIndex].foundDistance,newPos[stateIndex].to)<0) found=0;
				}
				if(newPos[stateIndex].finalDistance!=0)slide(newPos[stateIndex].finalDistance,newPos[stateIndex].finalState);
				return 1;
			}
		}
		printf("Slide in unknown position\n\r");
		fflush(stdout);
		return -1;
	}

	printf("Need TO state\n\r");
	fflush(stdout);
	return -1;
}


int slideSet(char *optionline)
{
	int result;
	int acceleration;
	int plateau;
	int drift;
	int driftPace;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&acceleration, &plateau, &drift, &driftPace);
	if (result==4)
	{
		TIM2CC3set(acceleration,plateau,drift,driftPace);
	}

	return 1;
}

int slideGet(char *optionline)
{
	motionstruct getstatus;
	TIM2CC3status(&getstatus);
	printf("Astep = %d\n\r",getstatus.asteps);
	printf("Plateau = %d\n\r",getstatus.pace);
	printf("Drift = %d\n\r",getstatus.drift);
	printf("Drift Pace = %d\n\r",getstatus.driftPace);
	printf("step = %d\n\r",getstatus.step);
	fflush(stdout);
	return 1;
}

int slideOptionNotFound(char *cmdline)
{
    printf("Unrecognized slide option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef slidestruct;

slidestruct slideoptions[]=
{
    {"position",sizeof("position")-1,slidePosition,""},
    {"move",sizeof("move")-1,slideMove,"Distance Pace"},
    {"pos",sizeof("pos")-1,slideMoveToPosition,"PosNum"},
    {"state",sizeof("state")-1,slideToState,"State Distance"},
    {"set",sizeof("set")-1,slideSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,slideGet,""},
    {"help",sizeof("help")-1,slideHelp,""},
    {"",0,slideOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(slideoptions)/sizeof(slidestruct))-1)

int slideHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",slideoptions[optionindex].option,slideoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int slidecommand(char *optionline)
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
				if(optlen==slideoptions[optindex].optionlen)
				{
					if(memcmp(opt,slideoptions[optindex].option,optlen)==0)
						{
						slideoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) slideoptions[optindex].optionfunc(optionline);
		}
		else slidePosition("slide position");
	return 0;
}

#undef OPTIONNUM

/*
		switch(currentPosition)
		{
		case PICKSTATE:
			if(newPosition==pickPos[0].number)
			{
				positionState=pickPos[0].to;
				positionDistance=pickPos[0].distance;
				finalState=pickPos[0].finalState;
				finalDistance=pickPos[0].finalDistance;
			}
			else if(newPosition==pickPos[1].number)
			{
				positionState=pickPos[1].to;
				positionDistance=pickPos[1].distance;
				finalState=pickPos[1].finalState;
				finalDistance=pickPos[1].finalDistance;
			}
			else if(newPosition==pickPos[2].number)
			{
				positionState=pickPos[2].to;
				positionDistance=pickPos[2].distance;
				finalState=pickPos[2].finalState;
				finalDistance=pickPos[2].finalDistance;
			}
			else
			{
					printf("Bad TO position\n\r");
					fflush(stdout);
				}
			break;
		case FLUSHSTATE:
			if(newPosition==flushPos[0].number)
			{
				positionState=flushPos[0].to;
				positionDistance=flushPos[0].distance;
				finalState=flushPos[0].finalState;
				finalDistance=flushPos[0].finalDistance;
			}
			else if(newPosition==flushPos[1].number)
			{
				positionState=flushPos[1].to;
				positionDistance=flushPos[1].distance;
				finalState=flushPos[1].finalState;
				finalDistance=flushPos[1].finalDistance;
			}
			else if(newPosition==flushPos[2].number)
			{
				positionState=flushPos[2].to;
				positionDistance=flushPos[2].distance;
				finalState=flushPos[2].finalState;
				finalDistance=flushPos[2].finalDistance;
			}
			else
			{
					printf("Bad TO position\n\r");
					fflush(stdout);
			}
			if(positionDistance!=0)
			break;
		case FILTERSTATE:
			if(newPosition==filterPos[0].number)
			{
				positionState=filterPos[0].to;
				positionDistance=filterPos[0].distance;
				finalState=filterPos[0].finalState;
				finalDistance=filterPos[0].finalDistance;
			}
			else if(newPosition==filterPos[1].number)
			{
				positionState=filterPos[1].to;
				positionDistance=filterPos[1].distance;
				finalState=filterPos[1].finalState;
				finalDistance=filterPos[1].finalDistance;
			}
			else if(newPosition==filterPos[2].number)
			{
				positionState=filterPos[2].to;
				positionDistance=filterPos[2].distance;
				finalState=filterPos[2].finalState;
				finalDistance=filterPos[2].finalDistance;
			}
			else
			{
					printf("Bad TO position\n\r");
					fflush(stdout);
			}
			break;
		case DROPSTATE:
			if(newPosition==dropPos[0].number)
			{
				positionState=dropPos[0].to;
				positionDistance=dropPos[0].distance;
				finalState=dropPos[0].finalState;
				finalDistance=dropPos[0].finalDistance;
			}
			else if(newPosition==dropPos[1].number)
			{
				positionState=dropPos[1].to;
				positionDistance=dropPos[1].distance;
				finalState=dropPos[1].finalState;
				finalDistance=dropPos[1].finalDistance;
			}
			else if(newPosition==dropPos[2].number)
			{
				positionState=dropPos[2].to;
				positionDistance=dropPos[2].distance;
				finalState=dropPos[2].finalState;
				finalDistance=dropPos[2].finalDistance;
			}
			else
			{
					printf("Bad TO position\n\r");
					fflush(stdout);
			}
			break;
		}
		*/
