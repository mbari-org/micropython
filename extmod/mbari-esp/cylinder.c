/*
 * cylinder.c
 *
 *  Created on: Aug 24, 2022
 *      Author: sjensen
 */


#include "cylinder.h"

int cylinderHelp(char *optionline);
unsigned int isqrt ( unsigned int s );

//steppers are 200 steps per revolution
//microstep at 16
//microsteps per revolution is 3200
//motor gear 16 tooth
//cylinder gear is 66 tooth
//microsteps around = 3200*66/16 = 13200
//12 tubes is 13200/12 = 1100 per tube
//cylinder could be anywhere within the 1100 steps of a tube
//therefore each move should add 1 full tube to make sure it can make it to
//its final position
//steps = (number of tubes to move + 1)*1100

#define PERTUBESTEPS 1100
#define DEFAULT_CYLINDER_STEPS_AROUND 15000

struct tube
{
int number;
int at;
int up;
int down;
} typedef tubedef;

tubedef tubes[] =
{
	{1,0x24,0x2c,0x34},
	{2,0x2c,0x2d,0x24},
	{3,0x2d,0x0d,0x2c},
	{4,0x0d,0x09,0x2d},
	{5,0x09,0x0b,0x0d},
	{6,0x0b,0x1b,0x09},
	{7,0x1b,0x13,0x0b},
	{8,0x13,0x12,0x1b},
	{9,0x12,0x32,0x13},
	{10,0x32,0x36,0x12},
	{11,0x36,0x34,0x32},
	{12,0x34,0x24,0x36},
};

#define NUMTUBES ((sizeof(tubes))/sizeof(tubedef))

int cylinderInit(void)
{

//Configfure GPIOC0 as output to be used as direction
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOCEN; //enable clock to gpioc;
	GPIOC->MODER	&= ~GPIO_MODER_MODE0;  //clear out mode
	GPIOC->MODER	|=	GPIO_MODER_MODE0_0; //set as output
	GPIOC->OTYPER &=~GPIO_OTYPER_OT0;  //set as push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0; //set as low speed
	GPIOC->BSRR = GPIO_BSRR_BR0; //clear GPIOC output

//Configure Timer2 Capture 1 on PA5
	TIM2CC1Initialize();

//configure GPIOE2-7 as input
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOEEN; //enable clock to gpioc;
	GPIOE->MODER	&= ~(GPIO_MODER_MODE2|GPIO_MODER_MODE3|GPIO_MODER_MODE4|GPIO_MODER_MODE5|GPIO_MODER_MODE6|GPIO_MODER_MODE7);  //clear out mode to make input

	return 1;
}

int readCylinderState(void)
{
	unsigned int state;
	state = (GPIOE->IDR&(CYLINDER_MASK<<2))>>2;
	return state;
}

int cylinderPosition(char *cmdline)
{
	unsigned int state;
	int result;
	char arguments[256];

	result=sscanf(cmdline,"%*s %*s %s",arguments);
	if(result>0)
	{
		printf("Extra arguments ignored\n\r");
	}
	state = (GPIOE->IDR&(CYLINDER_MASK<<2))>>2;
	printf("Cylinder state = %x (hex)\n\r",state);
	for(result=0;result<NUMTUBES;result++)
	{
		if(tubes[result].at==state)
		{
			printf("Cylinder at Tube %d\n\r",tubes[result].number);
		}
	}
	fflush(stdout);

	return 1;
}

signed char cylinder(int distance, unsigned int state)
{
	motionstruct cylinderMoveState;

	if (distance < 0)
	{
		distance=-distance;
		GPIOC->BSRR = GPIO_BSRR_BR0; //clear GPIOC output for reverse
	}
	else
	{
		GPIOC->BSRR = GPIO_BSRR_BS0; //set GPIOC output for forward
	}

	TIM2CC1InitMove(distance, state);  //kick off the move

	do
	{
		if(getBreak())
		{
			TIM2CC1stop();
			return -1;
		}
		TIM2CC1status(&cylinderMoveState);
	}while(cylinderMoveState.running==1);

	if (cylinderMoveState.running <0 )
    {
    	printf("Error\n\r");
    	fflush(stdout);
    }
	else
	{
		printf("step = %d\n\r",cylinderMoveState.step);
		fflush(stdout);
	}

    return 1;
}

int cylinderMove(char *optionline)
{
    int distance;
    int result;

	result=sscanf(optionline,"%*s %*s %d",&distance);
	if (result==1)
	{
		if (distance == 0) return -1;
		cylinder(distance, CYLINDER_NOSTATE);  //kick off the move
	}
    return 1;
}

int cylinderTubeCommand(char *optionline)
{
	int tube;
	int distance;
	int currentTubeIndex;
	int currentTube;
	int toTubeIndex;
	int currentState;
	int tubediff;
	int i;
	int toState;
	int result;
	int finalstate;
	int finaldistance;
	int initialstate;
	int initialdistance;

	result=sscanf(optionline,"%*s %*s %d",&tube);
	if (result<1)
	{
		printf("Tube required\n\r");
		fflush(stdout);
		return -1;
	}
	if (result==1 )
	{
	if((tube <1)||(tube>12))
	{
		printf("valid tube from 1-12\n\r");
		fflush(stdout);
		return -2;
	}
	currentState= readCylinderState();
	toTubeIndex=-1;
	currentTubeIndex=-1;
	for(i=0;i<NUMTUBES;i++)
	{
		if(tubes[i].number==tube)
		{
			toTubeIndex=i;
		}
		if(tubes[i].at==currentState)
		{
			currentTubeIndex=i;
			currentTube=tubes[i].number;
		}
	}
	if (toTubeIndex <0) return -3;
	if (currentTubeIndex<0) return -4;

	initialdistance=0;

	tubediff=tube-currentTube;

 	if(tubediff<-6)
		{
			distance = -(PERTUBESTEPS);
			tubediff = 12+tubediff;
			toState = tubes[toTubeIndex].at;
			finalstate=tubes[toTubeIndex].at;
			finaldistance=0;

		}
	else if(tubediff<1)
		{
		distance = (PERTUBESTEPS);
		tubediff=-tubediff;
			toState = tubes[toTubeIndex].down;
			finalstate=tubes[toTubeIndex].at;
			initialdistance=(PERTUBESTEPS);
			initialstate==tubes[currentTubeIndex].down;
			finaldistance=-(PERTUBESTEPS);
		}
	else if(tubediff<7)
		{
		distance=-(PERTUBESTEPS);
			toState = tubes[toTubeIndex].at;
			finalstate=tubes[toTubeIndex].at;
			finaldistance=0;
		}
	else
		{
		distance=(PERTUBESTEPS);
		tubediff = 12-tubediff;
			toState = tubes[toTubeIndex].down;
			finalstate=tubes[toTubeIndex].at;
			finaldistance=-(PERTUBESTEPS);
		}

 	distance=distance*(tubediff);
 	printf("steps = %d\n\r",distance);
 	fflush(stdout);
 	if(initialdistance!=0) cylinder(initialdistance,initialstate);
	cylinder(distance, toState);  //kick off the move
    if(finaldistance!=0)
    {
    	cylinder(finaldistance,finalstate);
    }
	return 1;
	}
	return -5;
}

int cylinderSet(char *optionline)
{
	int result;
	int acceleration;
	int plateau;
	int drift;
	int driftPace;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&acceleration, &plateau, &drift, &driftPace);
	if (result==4)
	{
		TIM2CC1set(acceleration,plateau,drift,driftPace);
	}

	return 1;
}

int cylinderGet(char *optionline)
{
	motionstruct getstatus;
	TIM2CC1status(&getstatus);
	printf("Astep = %d\n\r",getstatus.asteps);
	printf("Plateau = %d\n\r",getstatus.pace);
	printf("Drift = %d\n\r",getstatus.drift);
	printf("Drift Pace = %d\n\r",getstatus.driftPace);
	fflush(stdout);
	return 1;
}

int cylinderTraj(char *optionline)
{
    int distance;
    unsigned int pace;
    int result;
	motionstruct trajsim;
	int steps;
	int stepatglide=0;
	int glide=100;  //glide distance is +/- this value.  starts gliding -glide before until +glide after
	int stepatoffset=0;
	int simstate=tubes[4].at;
	int offsetstate;
	int adjust=0;
	int gsteps;
	int startTube, endTube, startOffset, endOffset;

	result=sscanf(optionline,"%*s %*s %d %d %d %d",&startTube, &endTube, &startOffset, &endOffset);
			//startOffest is how far away from the true position is the cylinder, miss aligned
			//endOffset is how far off is perfect tube alignment
	if (result==4)
	{
		if ((startTube<1)||(startTube>12)||(endTube<1)||(endTube>12)||(startTube>=endTube)) return -1;

//		trajsim.toState=toState;
		TIM2CC1status(&trajsim);

		glide=trajsim.drift;
		distance=(endTube-startTube)*1100;

		trajsim.toState=tubes[endTube].at; //toState; //This is where we want to be "at"
		offsetstate=tubes[startTube].up;  //since assuming moving "up" this is the next transition, must be more than one tube away
		trajsim.running=STEPRUNNING;
		trajsim.steps=distance+glide;   //only positive for now and overshoot, this will be updates along the way

		if(trajsim.drift>=trajsim.steps) //check if this is just a drift move
		{//drift move
			trajsim.usteps=0;
			trajsim.dsteps=trajsim.steps;
			trajsim.P=trajsim.driftPace;  //so set the pace equal to the drift pace
			trajsim.step=1;
		}

		else
		{
		trajsim.usteps=trajsim.asteps;
		trajsim.step=1;
		trajsim.C = (int) isqrt(trajsim.usteps);
		trajsim.C *= trajsim.pace;
		steps=(trajsim.usteps*2)+trajsim.drift;
		if(steps>trajsim.steps) //ramp up down drift is longer than the move
		{
			trajsim.usteps=(trajsim.steps-trajsim.drift)/2;  //now ustep is half of the total move
			trajsim.dsteps=trajsim.usteps; //switch from astep to dstep
		}
		else trajsim.dsteps=trajsim.steps-trajsim.usteps;//-trajsim.drift;
		trajsim.start = trajsim.C;
		trajsim.finish = trajsim.start/2;
		trajsim.delta = trajsim.finish;
		trajsim.numer = 0;
		trajsim.odd = 3;
		trajsim.intsqrt=1;
		trajsim.intsqrtmod=0;
		trajsim.numer=trajsim.C*trajsim.odd;
		trajsim.start= trajsim.intsqrt*trajsim.odd;
	    trajsim.P=trajsim.numer/trajsim.start;
		}
	    if(trajsim.drift==0) trajsim.driftPace = 0x7fffffff; //default really slow since not used

		trajsim.HL=0; //sync the indicator flag
		trajsim.counter = 0;
		trajsim.counter += trajsim.P;

		do {
				if(trajsim.step==startOffset) simstate=tubes[startTube].up;//simulate a change of state
				if(trajsim.step==distance+glide+endOffset) simstate=tubes[endTube].at;//simulate a change of state

				if(stepatoffset==0)
					if(simstate==offsetstate) stepatoffset=(1100-trajsim.step);  //where the next state was found

				if(adjust==0)
					if((stepatoffset!=0)&&(stepatglide!=0)) //time to do some adjustments
					{
					trajsim.steps-=stepatoffset;
					trajsim.dsteps-=(stepatoffset+glide+glide-stepatglide);
					adjust=1;
					}
				  if(trajsim.step<(trajsim.usteps+1)) //accelerating up
				  {

				  trajsim.P=trajsim.numer/(trajsim.start+trajsim.intsqrtmod);// trajsim.delta;
				    trajsim.intsqrtmod++;   //next modulus step up
				    if (trajsim.intsqrtmod>=trajsim.odd)  //is it time to reset and step to next square root
				        {
				    	trajsim.intsqrt++;	//next square root
				    	trajsim.intsqrtmod=0;  //reset modulus
				    	trajsim.odd+=2;  //numer of integers before change in sqrt

						trajsim.start= trajsim.intsqrt*trajsim.odd;
					    trajsim.numer=trajsim.C*trajsim.odd;

				        }

				  }
				  if((trajsim.step>trajsim.dsteps)&&(trajsim.P<trajsim.driftPace))//accelerating down to drift, reverses acceleration from above
				  {
					if(trajsim.intsqrt==0)
						trajsim.P=trajsim.driftPace;
					else trajsim.P=trajsim.numer/(trajsim.start+trajsim.intsqrtmod);//trajsim.delta;

				    if (trajsim.intsqrtmod==0)
				        {
				    	trajsim.intsqrt--;
				    	trajsim.odd-=2;
				    	trajsim.intsqrtmod=trajsim.odd;

						trajsim.start= trajsim.intsqrt*trajsim.odd;
					    trajsim.numer=trajsim.C*trajsim.odd;
				    	trajsim.intsqrtmod=trajsim.odd;

				        }
				    trajsim.intsqrtmod--;
				  }
				  //not accelerating up or down thus P stays the same
				    trajsim.counter += trajsim.P;  //now move counter one full pace ahead
					trajsim.HL=0;
					trajsim.step++;
					if (trajsim.toState == simstate)// (GPIOE->IDR&(CYLINDER_MASK<<2))>>2)  //Edge found, freeze and flag done
					{
						trajsim.running=STEPDONE;  //leave steps non zero for diagnostic purposes
					}
					if (trajsim.step>=trajsim.steps) //no edge found and steps are done, freeze and flag error
					{
						if(trajsim.toState != CYLINDER_NOSTATE) trajsim.running=STEPPEDOUT; //trying to find state and ran out of steps
						else trajsim.running=STEPDONE; //no trying to find state
					}
					if (stepatglide==0)
						if(trajsim.P<trajsim.driftPace) stepatglide=trajsim.step;
					printf("%d,%d,%d,%d,%d,%d\n\r",trajsim.step,trajsim.P,trajsim.steps,trajsim.dsteps,stepatglide,stepatoffset);

		}
		while (trajsim.running==STEPRUNNING);

//		TIM2CC1InitMove(distance, pace, state);  //kick off the move

		//cylinder(distance, pace, CYLINDER_NOSTATE);  //kick off the move
	}
    return 1;
}



int cylinderOptionNotFound(char *cmdline)
{
    printf("Unrecognized cylinder option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef cylinderstruct;

cylinderstruct cylinderoptions[]=
{
    {"position",sizeof("position")-1,cylinderPosition,""},
    {"move",sizeof("move")-1,cylinderMove,"Distance Pace"},
    {"tube",sizeof("tube")-1,cylinderTubeCommand,"Tube Pace"},
    {"set",sizeof("set")-1,cylinderSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,cylinderGet,""},
    {"traj",sizeof("traj")-1,cylinderTraj,"Distance Pace"},
    {"help",sizeof("help")-1,cylinderHelp,""},
    {"",0,cylinderOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(cylinderoptions)/sizeof(cylinderstruct))-1)

int cylinderHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",cylinderoptions[optionindex].option,cylinderoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int cylindercommand(char *optionline)
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
				if(optlen==cylinderoptions[optindex].optionlen)
				{
					if(memcmp(opt,cylinderoptions[optindex].option,optlen)==0)
						{
						cylinderoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) cylinderoptions[optindex].optionfunc(optionline);
		}
		else cylinderPosition("cylinder position");
	return 0;
}

#undef OPTIONNUM
