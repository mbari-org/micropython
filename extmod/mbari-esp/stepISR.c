/*
 * stepISR.c
 *
 *  Created on: Aug 11, 2022
 *      Author: sjensen
 */
#include "stepISR.h"
#include "carousel.h"

//stepstruct tim2cc1;
motionstruct tim2cc1;
motionstruct tim2cc3;
motionstruct tim2cc4;
motionstruct tim5cc2;
motionstruct tim5cc3;

unsigned int isqrt ( unsigned int s )
{
//	long long x0 = s / 2;			// Initial estimate

unsigned int shiftr=s;
unsigned int power;
unsigned int x0;
unsigned int x1;

for(power=1;((power<32)&&(shiftr!=0));power++)
{
    shiftr=shiftr>>1;
}
//printf("\r\npower = %d\r\n",power);
//fflush(stdout);

	x0 = (1<<(power>>1));  //Avoid overflow when s is the maximum representable value

	// Sanity check
	if ( x0 != 0 )
	{
		x1 = ( x0 + s / x0 ) / 2;	// Update

		while ( x1 < x0 )				// This also checks for cycle
		{
//            printf("%d\n\r",x0);fflush(stdout);
			x0 = x1;
			x1 = ( x0 + s / x0 ) / 2;
		}

		return x0;
	}
	else
	{
		return s;
	}
}


void TIM2_IRQHandler(void)
{
	if(TIM2->DIER&TIM_DIER_CC1IE)
		  TIM2->SR = ~(TIM_SR_CC1IF);  //clear the flag
		  if(tim2cc1.HL)
		  {//just toggled low
			  if(tim2cc1.step<(tim2cc1.usteps+1)) //accelerating up
			  {

			  tim2cc1.P=tim2cc1.numer/(tim2cc1.start+tim2cc1.intsqrtmod);// tim2cc1.delta;
			    tim2cc1.intsqrtmod++;   //next modulus step up
			    if (tim2cc1.intsqrtmod>=tim2cc1.odd)  //is it time to reset and step to next square root
			        {
			    	tim2cc1.intsqrt++;	//next square root
			    	tim2cc1.intsqrtmod=0;  //reset modulus
			    	tim2cc1.odd+=2;  //numer of integers before change in sqrt

					tim2cc1.start= tim2cc1.intsqrt*tim2cc1.odd;
				    tim2cc1.numer=tim2cc1.C*tim2cc1.odd;

			        }

			  }
			  if((tim2cc1.step>tim2cc1.dsteps)&&(tim2cc1.P<tim2cc1.driftPace))//accelerating down to drift, reverses acceleration from above
			  {
				if(tim2cc1.intsqrt==0)
					tim2cc1.P=tim2cc1.driftPace;
				else tim2cc1.P=tim2cc1.numer/(tim2cc1.start+tim2cc1.intsqrtmod);//tim2cc1.delta;

			    if (tim2cc1.intsqrtmod==0)
			        {
			    	tim2cc1.intsqrt--;
			    	tim2cc1.odd-=2;
			    	tim2cc1.intsqrtmod=tim2cc1.odd;

					tim2cc1.start= tim2cc1.intsqrt*tim2cc1.odd;
				    tim2cc1.numer=tim2cc1.C*tim2cc1.odd;
			    	tim2cc1.intsqrtmod=tim2cc1.odd;

			        }
			    tim2cc1.intsqrtmod--;
			  }
			  //not accelerating up or down thus P stays the same
			    tim2cc1.counter += tim2cc1.P;  //now move counter one full pace ahead
				TIM2->CCR1 = tim2cc1.counter;  //time to next rising edge
				tim2cc1.HL=0;
				tim2cc1.step++;
				if (tim2cc1.toState == (GPIOE->IDR&(CYLINDER_MASK<<2))>>2)  //Edge found, freeze and flag done
				{
					TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
					TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 1 interrupt
					tim2cc1.running=STEPDONE;  //leave steps non zero for diagnostic purposes
				}
				if (tim2cc1.step>=tim2cc1.steps) //no edge found and steps are done, freeze and flag error
				{
					TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
					TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 1 interrupt
					if(tim2cc1.toState != CYLINDER_NOSTATE) tim2cc1.running=STEPPEDOUT; //trying to find state and ran out of steps
					else tim2cc1.running=STEPDONE; //no trying to find state
				}
		  }
		  else
		  {//just toggled high
				TIM2->CCR1 = tim2cc1.counter+highincrement;  //time to be high which will not effect pace
				tim2cc1.HL=1;
		  }

	if(TIM2->DIER&TIM_DIER_CC3IE)
	  if((TIM2->SR)&TIM_SR_CC3IF)  //was the interrupt from output compare 3
	  {
		  TIM2->SR = ~(TIM_SR_CC3IF);  //clear the flag
		  if(tim2cc3.HL)
		  {//just toggled low
			  if(tim2cc3.step<(tim2cc3.usteps+1)) //accelerating up
			  {

			  tim2cc3.P=tim2cc3.numer/(tim2cc3.start+tim2cc3.intsqrtmod);// tim2cc3.delta;
			    tim2cc3.intsqrtmod++;   //next modulus step up
			    if (tim2cc3.intsqrtmod>=tim2cc3.odd)  //is it time to reset and step to next square root
			        {
			    	tim2cc3.intsqrt++;	//next square root
			    	tim2cc3.intsqrtmod=0;  //reset modulus
			    	tim2cc3.odd+=2;  //numer of integers before change in sqrt

					tim2cc3.start= tim2cc3.intsqrt*tim2cc3.odd;
				    tim2cc3.numer=tim2cc3.C*tim2cc3.odd;

			        }

			  }
			  if((tim2cc3.step>tim2cc3.dsteps)&&(tim2cc3.P<tim2cc3.driftPace))//accelerating down to drift, reverses acceleration from above
			  {
				if(tim2cc3.intsqrt==0)
					tim2cc3.P=tim2cc3.driftPace;
				else tim2cc3.P=tim2cc3.numer/(tim2cc3.start+tim2cc3.intsqrtmod);//tim2cc3.delta;

			    if (tim2cc3.intsqrtmod==0)
			        {
			    	tim2cc3.intsqrt--;
			    	tim2cc3.odd-=2;
			    	tim2cc3.intsqrtmod=tim2cc3.odd;

					tim2cc3.start= tim2cc3.intsqrt*tim2cc3.odd;
				    tim2cc3.numer=tim2cc3.C*tim2cc3.odd;
			    	tim2cc3.intsqrtmod=tim2cc3.odd;

			        }
			    tim2cc3.intsqrtmod--;
			  }
			  //not accelerating up or down thus P stays the same
			    tim2cc3.counter += tim2cc3.P;  //now move counter one full pace ahead
				TIM2->CCR3 = tim2cc3.counter;  //time to next rising edge
				tim2cc3.HL=0;
				tim2cc3.step++;
				if (tim2cc3.toState == (GPIOE->IDR&(SLIDE_MASK<<8))>>8)  //Edge found, freeze and flag done
				{
					if(++tim2cc3.matchcount==10)
					{
						TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
						TIM2->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 3 interrupt
						tim2cc3.running=STEPDONE;  //leave steps non zero for diagnostic purposes
					}
				}
				else tim2cc3.matchcount=0;
				if (tim2cc3.step>=tim2cc3.steps) //no edge found and steps are done, freeze and flag error
				{
					TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
					TIM2->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 3 interrupt
					if(tim2cc3.toState != SLIDE_NOSTATE) tim2cc3.running=STEPPEDOUT; //trying to find state and ran out of steps
					else tim2cc3.running=STEPDONE; //no trying to find state
				}
		  }
		  else
		  {//just toggled high
				TIM2->CCR3 = tim2cc3.counter+highincrement;  //time to be high which will not effect pace
				tim2cc3.HL=1;
		  }

	  }

	if(TIM2->DIER&TIM_DIER_CC4IE)
	  if((TIM2->SR)&TIM_SR_CC4IF)  //was the interrupt from output compare 4
	  {
		  TIM2->SR = ~(TIM_SR_CC4IF);  //clear the flag
		  if(tim2cc4.HL)
		  {//just toggled low
			  if(tim2cc4.step<(tim2cc4.usteps+1)) //accelerating up
			  {

			  tim2cc4.P=tim2cc4.numer/(tim2cc4.start+tim2cc4.intsqrtmod);// tim2cc4.delta;
			    tim2cc4.intsqrtmod++;   //next modulus step up
			    if (tim2cc4.intsqrtmod>=tim2cc4.odd)  //is it time to reset and step to next square root
			        {
			    	tim2cc4.intsqrt++;	//next square root
			    	tim2cc4.intsqrtmod=0;  //reset modulus
			    	tim2cc4.odd+=2;  //numer of integers before change in sqrt

					tim2cc4.start= tim2cc4.intsqrt*tim2cc4.odd;
				    tim2cc4.numer=tim2cc4.C*tim2cc4.odd;

			        }

			  }
			  if((tim2cc4.step>tim2cc4.dsteps)&&(tim2cc4.P<tim2cc4.driftPace))//accelerating down to drift, reverses acceleration from above
			  {
				if(tim2cc4.intsqrt==0)
					tim2cc4.P=tim2cc4.driftPace;
				else tim2cc4.P=tim2cc4.numer/(tim2cc4.start+tim2cc4.intsqrtmod);//tim2cc4.delta;

			    if (tim2cc4.intsqrtmod==0)
			        {
			    	tim2cc4.intsqrt--;
			    	tim2cc4.odd-=2;
			    	tim2cc4.intsqrtmod=tim2cc4.odd;

					tim2cc4.start= tim2cc4.intsqrt*tim2cc4.odd;
				    tim2cc4.numer=tim2cc4.C*tim2cc4.odd;
			    	tim2cc4.intsqrtmod=tim2cc4.odd;

			        }
			    tim2cc4.intsqrtmod--;
			  }
			  //not accelerating up or down thus P stays the same
			    tim2cc4.counter += tim2cc4.P;  //now move counter one full pace ahead
				TIM2->CCR4 = tim2cc4.counter;  //time to next rising edge
				tim2cc4.HL=0;
				tim2cc4.step++;
				{
				unsigned int state;
				state=GPIOE->IDR;
				state = ((state&(0x3<<14))>>12)|(state&0x3);
				if (tim2cc4.toState == state)  //Edge found, freeze and flag done
				{
					TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
					TIM2->DIER&=~TIM_DIER_CC4IE;  //disable compare channel 3 interrupt
					tim2cc4.running=STEPDONE;  //leave steps non zero for diagnostic purposes
				}
				}
				if (tim2cc4.step>=tim2cc4.steps) //no edge found and steps are done, freeze and flag error
				{
					TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
					TIM2->DIER&=~TIM_DIER_CC4IE;  //disable compare channel 3 interrupt
					if(tim2cc4.toState != SLIDE_NOSTATE) tim2cc4.running=STEPPEDOUT; //trying to find state and ran out of steps
					else tim2cc4.running=STEPDONE; //no trying to find state
				}
		  }
		  else
		  {//just toggled high
				TIM2->CCR4 = tim2cc4.counter+highincrement;  //time to be high which will not effect pace
				tim2cc4.HL=1;
		  }
	  }

}

void TIM5_IRQHandler(void)
{
	if(TIM5->DIER&TIM_DIER_CC2IE)
	  if((TIM5->SR)&TIM_SR_CC2IF)  //was the interrupt from output compare 1
	  {
		  TIM5->SR = ~(TIM_SR_CC2IF);  //clear the flag
		  if(tim5cc2.HL)
		  {//just toggled low
			  if(tim5cc2.step<(tim5cc2.usteps+1)) //accelerating up
			  {

			  tim5cc2.P=tim5cc2.numer/(tim5cc2.start+tim5cc2.intsqrtmod);// tim2cc3.delta;
			    tim5cc2.intsqrtmod++;   //next modulus step up
			    if (tim5cc2.intsqrtmod>=tim5cc2.odd)  //is it time to reset and step to next square root
			        {
			    	tim5cc2.intsqrt++;	//next square root
			    	tim5cc2.intsqrtmod=0;  //reset modulus
			    	tim5cc2.odd+=2;  //numer of integers before change in sqrt

					tim5cc2.start= tim5cc2.intsqrt*tim5cc2.odd;
				    tim5cc2.numer=tim5cc2.C*tim5cc2.odd;

			        }

			  }
			  if((tim5cc2.step>tim5cc2.dsteps)&&(tim5cc2.P<tim5cc2.driftPace))//accelerating down to drift, reverses acceleration from above
			  {
				if(tim5cc2.intsqrt==0)
					tim5cc2.P=tim5cc2.driftPace;
				else tim5cc2.P=tim5cc2.numer/(tim5cc2.start+tim5cc2.intsqrtmod);//tim2cc3.delta;

			    if (tim5cc2.intsqrtmod==0)
			        {
			    	tim5cc2.intsqrt--;
			    	tim5cc2.odd-=2;
			    	tim5cc2.intsqrtmod=tim5cc2.odd;

					tim5cc2.start= tim5cc2.intsqrt*tim5cc2.odd;
				    tim5cc2.numer=tim5cc2.C*tim5cc2.odd;
			    	tim5cc2.intsqrtmod=tim5cc2.odd;

			        }
			    tim5cc2.intsqrtmod--;
			  }
			  //not accelerating up or down thus P stays the same
			    tim5cc2.counter += tim5cc2.P;  //now move counter one full pace ahead
				TIM5->CCR2 = tim5cc2.counter;  //time to next rising edge
				tim5cc2.HL=0;
				tim5cc2.step++;
				if (tim5cc2.toState == (GPIOE->IDR&(SLIDE_MASK<<8))>>8)  //Edge found, freeze and flag done
				{
					TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
					TIM5->DIER&=~TIM_DIER_CC2IE;  //disable compare channel 1 interrupt
					tim5cc2.running=STEPDONE;  //leave steps non zero for diagnostic purposes
				}
				if (tim5cc2.step>=tim5cc2.steps) //no edge found and steps are done, freeze and flag error
				{
					TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
					TIM5->DIER&=~TIM_DIER_CC2IE;  //disable compare channel 1 interrupt
					if(tim5cc2.toState != SLIDE_NOSTATE) tim5cc2.running=STEPPEDOUT; //trying to find state and ran out of steps
					else tim5cc2.running=STEPDONE; //no trying to find state
				}
		  }
		  else
		  {//just toggled high
				TIM5->CCR2 = tim5cc2.counter+highincrement;  //time to be high which will not effect pace
				tim5cc2.HL=1;
		  }
	  }

  if(TIM5->DIER&TIM_DIER_CC3IE)
	if((TIM5->SR)&TIM_SR_CC3IF)  //was the interrupt from output compare 1
	  {
		  TIM5->SR = ~(TIM_SR_CC3IF);  //clear the flag
		  if(tim5cc3.HL)
		  {//just toggled low
			  if(tim5cc3.step<(tim5cc3.usteps+1)) //accelerating up
			  {

			  tim5cc3.P=tim5cc3.numer/(tim5cc3.start+tim5cc3.intsqrtmod);// tim2cc3.delta;
			    tim5cc3.intsqrtmod++;   //next modulus step up
			    if (tim5cc3.intsqrtmod>=tim5cc3.odd)  //is it time to reset and step to next square root
			        {
			    	tim5cc3.intsqrt++;	//next square root
			    	tim5cc3.intsqrtmod=0;  //reset modulus
			    	tim5cc3.odd+=2;  //numer of integers before change in sqrt

					tim5cc3.start= tim5cc3.intsqrt*tim5cc3.odd;
				    tim5cc3.numer=tim5cc3.C*tim5cc3.odd;

			        }

			  }
			  if((tim5cc3.step>tim5cc3.dsteps)&&(tim5cc3.P<tim5cc3.driftPace))//accelerating down to drift, reverses acceleration from above
			  {
				if(tim5cc3.intsqrt==0)
					tim5cc3.P=tim5cc3.driftPace;
				else tim5cc3.P=tim5cc3.numer/(tim5cc3.start+tim5cc3.intsqrtmod);//tim2cc3.delta;

			    if (tim5cc3.intsqrtmod==0)
			        {
			    	tim5cc3.intsqrt--;
			    	tim5cc3.odd-=2;
			    	tim5cc3.intsqrtmod=tim5cc3.odd;

					tim5cc3.start= tim5cc3.intsqrt*tim5cc3.odd;
				    tim5cc3.numer=tim5cc3.C*tim5cc3.odd;
			    	tim5cc3.intsqrtmod=tim5cc3.odd;

			        }
			    tim5cc3.intsqrtmod--;
			  }
			  //not accelerating up or down thus P stays the same
			    tim5cc3.counter += tim5cc3.P;  //now move counter one full pace ahead
				TIM5->CCR3 = tim5cc3.counter;  //time to next rising edge
				tim5cc3.HL=0;
				tim5cc3.step++;
				if (tim5cc3.toState == (GPIOE->IDR&(SLIDE_MASK<<8))>>8)  //Edge found, freeze and flag done
				{
					TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
					TIM5->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 1 interrupt
					tim5cc3.running=STEPDONE;  //leave steps non zero for diagnostic purposes
				}
				if (tim5cc3.step>=tim5cc3.steps) //no edge found and steps are done, freeze and flag error
				{
					TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
					TIM5->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 1 interrupt
					if(tim5cc3.toState != SLIDE_NOSTATE) tim5cc3.running=STEPPEDOUT; //trying to find state and ran out of steps
					else tim5cc3.running=STEPDONE; //no trying to find state
				}
		  }
		  else
		  {//just toggled high
				TIM5->CCR3 = tim5cc3.counter+highincrement;  //time to be high which will not effect pace
				tim5cc3.HL=1;
		  }

	  }
}

void TIM2CC1Initialize(void)
{
//configure GPIOA5 to output using TIM2 CC1 as source as step
	#define TIM2_AF 0x01
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOAEN; //enable clock to gpioa;
	GPIOA->MODER	|=	GPIO_MODER_MODE5_1;
	GPIOA->MODER	&= ~GPIO_MODER_MODE5_0;
	GPIOA->AFR[0] |= (TIM2_AF<<20);

	RCC->APB1ENR1  |= RCC_APB1ENR1_TIM2EN; //enable clock to tim2;
	TIM2->CR1     &=~ TIM_CR1_CEN;
	TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
	TIM2->CCMR1|=TIM_CCMR1_OC1M_2;  //force low, now!
	TIM2->CCER|=TIM_CCER_CC1E;  //enable compare channel 1
	TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 1 interrupt
	TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
	TIM2->CR1     |= TIM_CR1_CEN;  //enable counter

	tim2cc1.asteps=26400;  //setup defaults
	tim2cc1.pace=2000;
	tim2cc1.drift=1100;
	tim2cc1.driftPace=0x4000;

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim2_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM2_IRQn, tim2_pri_encoding );
    NVIC_EnableIRQ( TIM2_IRQn );
    return;
}

int TIM2CC1InitMove(int steps, int toState )
{
	TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 1 interrupt
	tim2cc1.toState=toState;

	TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 1 interrupt
	tim2cc1.toState=toState;
	tim2cc1.running=STEPRUNNING;
	tim2cc1.steps=steps;
	if(tim2cc1.drift>=steps) //check if this is just a drift move
	{//drift move
		tim2cc1.usteps=0;
		tim2cc1.dsteps=steps;
		tim2cc1.P=tim2cc1.driftPace;  //so set the pace equal to the drift pace
		tim2cc1.step=1;
	}
	else
	{
	tim2cc1.usteps=tim2cc1.asteps;
	tim2cc1.step=1;
	tim2cc1.C = (int) isqrt(tim2cc1.usteps);
	tim2cc1.C *= tim2cc1.pace;
	steps=(tim2cc1.usteps*2)+tim2cc1.drift;
	if(steps>tim2cc1.steps) //ramp up down drift is longer than the move
	{
		tim2cc1.usteps=(tim2cc1.steps-tim2cc1.drift)/2;  //now ustep is half of the total move
		tim2cc1.dsteps=tim2cc1.usteps; //switch from astep to dstep
	}
	else tim2cc1.dsteps=tim2cc1.steps-tim2cc1.usteps-tim2cc1.drift;
	tim2cc1.start = tim2cc1.C;
	tim2cc1.finish = tim2cc1.start/2;
	tim2cc1.delta = tim2cc1.finish;
	tim2cc1.numer = 0;
	tim2cc1.odd = 3;
	tim2cc1.intsqrt=1;
	tim2cc1.intsqrtmod=0;
	tim2cc1.numer=tim2cc1.C*tim2cc1.odd;
	tim2cc1.start= tim2cc1.intsqrt*tim2cc1.odd;
    tim2cc1.P=tim2cc1.numer/tim2cc1.start;
	}
    if(tim2cc1.drift==0) tim2cc1.driftPace = 0x7fffffff;

	TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
	TIM2->CCMR1|=TIM_CCMR1_OC1M_2;  //force low, now!
	tim2cc1.HL=0; //sync the indicator flag
	tim2cc1.counter = TIM2->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim2cc1.counter += tim2cc1.P;
	TIM2->CCR1 = tim2cc1.counter;  //set first transition count
	TIM2->SR &= ~(TIM_SR_CC1IF);//clear the interrupt flag
	TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
	TIM2->CCMR1|=TIM_CCMR1_OC1M_0|TIM_CCMR1_OC1M_1;  //set to toggle mode
	TIM2->DIER|=TIM_DIER_CC1IE;  //enable compare channel 1 interrupt
	return 1;

}

int TIM2CC1status(motionstruct *currentStatus)
{

	currentStatus->counter = tim2cc1.counter;
	currentStatus->HL = tim2cc1.HL;
	currentStatus->step = tim2cc1.step;
	currentStatus->toState = tim2cc1.toState;
	currentStatus->running = tim2cc1.running;
	currentStatus->asteps=tim2cc1.asteps;
	currentStatus->drift=tim2cc1.drift;
	currentStatus->driftPace=tim2cc1.driftPace;
	currentStatus->pace=tim2cc1.pace;

	return 1;
}

int TIM2CC1set(int acceleration,int plateau, int drift,int driftPace)
{
	tim2cc1.asteps=acceleration;
	tim2cc1.pace=plateau;
	tim2cc1.drift=drift;
	tim2cc1.driftPace=driftPace;
	return 1;
}

int TIM2CC1stop(void)
{
	TIM2->DIER&=~TIM_DIER_CC1IE;  //disable compare channel 3 interrupt
	TIM2->CCMR1&=~(TIM_CCMR1_OC1M); //clear it out to frozen mode
	TIM2->CCMR1|=(TIM_CCMR1_OC1M_2);  //force low, now!
	return 1;
}

void TIM2CC3Initialize(void)
{
//configure GPIOB10 output using TIM2 CC3 as source as step
	#define TIM2_AF 0x01
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOBEN; //enable clock to gpiob;
	GPIOB->MODER	|=	GPIO_MODER_MODE10_1;
	GPIOB->MODER	&= ~GPIO_MODER_MODE10_0;
	GPIOB->AFR[1] |= (TIM2_AF<<8);

	RCC->APB1ENR1  |= RCC_APB1ENR1_TIM2EN; //enable clock to tim2;
	TIM2->CR1     &=~ TIM_CR1_CEN;
	TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM2->CCMR2|=(TIM_CCMR2_OC3M_2);  //force low, now!
	TIM2->CCER|=TIM_CCER_CC3E;  //enable compare channel 3
	TIM2->DIER&=~TIM_DIER_CC3IE;  //enable compare channel 3 interrupt
	TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM2->CR1     |= TIM_CR1_CEN;  //enable counter

	tim2cc3.asteps=26400;  //setup defaults
	tim2cc3.pace=1024;
	tim2cc3.drift=3200;
	tim2cc3.driftPace=0x4000;

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim2_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM2_IRQn, tim2_pri_encoding );
    NVIC_EnableIRQ( TIM2_IRQn );
    return;
}

int TIM2CC3InitMove(int steps, int toState  )
{
	TIM2->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 1 interrupt
	tim2cc3.toState=toState;
	tim2cc3.running=STEPRUNNING;
	tim2cc3.steps=steps;
	tim2cc3.usteps=tim2cc3.asteps;
	tim2cc3.step=1;
	tim2cc3.C = (int) isqrt(tim2cc3.usteps);
	tim2cc3.C *= tim2cc3.pace;
	steps=(tim2cc3.usteps*2)+tim2cc3.drift;
	if(steps>tim2cc3.steps) //ramp up down drift is longer than the move
	{
		tim2cc3.usteps=(tim2cc3.steps-tim2cc3.drift)/2;  //now ustep is half of the total move
		tim2cc3.dsteps=tim2cc3.usteps; //switch from astep to dstep
	}
	else tim2cc3.dsteps=tim2cc3.steps-tim2cc3.usteps-tim2cc3.drift;
	tim2cc3.start = tim2cc3.C;
	tim2cc3.finish = tim2cc3.start/2;
	tim2cc3.delta = tim2cc3.finish;
	tim2cc3.numer = 0;
	tim2cc3.odd = 3;
	tim2cc3.intsqrt=1;
	tim2cc3.intsqrtmod=0;
	tim2cc3.numer=tim2cc3.C*tim2cc3.odd;
	tim2cc3.start= tim2cc3.intsqrt*tim2cc3.odd;
    tim2cc3.P=tim2cc3.numer/tim2cc3.start;
    if(tim2cc3.drift==0) tim2cc3.driftPace = 0x7fffffff;


	TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC3M_2;  //force low, now!
	tim2cc3.HL=0; //sync the indicator flag
	tim2cc3.counter = TIM2->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim2cc3.counter += tim2cc3.P;
	TIM2->CCR3 = tim2cc3.counter;  //set first transition count
	TIM2->SR &= ~(TIM_SR_CC3IF);//clear the interrupt flag
	TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC3M_0|TIM_CCMR2_OC3M_1;  //set to toggle mode
	TIM2->DIER|=TIM_DIER_CC3IE;  //enable compare channel 1 interrupt
	return 1;

}

int TIM2CC3status(motionstruct *currentStatus)
{

	currentStatus->counter = tim2cc3.counter;
	currentStatus->HL = tim2cc3.HL;
	currentStatus->step = tim2cc3.step;
	currentStatus->toState = tim2cc3.toState;
	currentStatus->running = tim2cc3.running;
	currentStatus->asteps=tim2cc3.asteps;
	currentStatus->drift=tim2cc3.drift;
	currentStatus->driftPace=tim2cc3.driftPace;
	currentStatus->pace=tim2cc3.pace;

	return 1;
}

int TIM2CC3set(int acceleration,int plateau, int drift,int driftPace)
{
	tim2cc3.asteps=acceleration;
	tim2cc3.pace=plateau;
	tim2cc3.drift=drift;
	tim2cc3.driftPace=driftPace;
	return 1;
}

int TIM2CC3stop(void)
{
	TIM2->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 3 interrupt
	TIM2->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM2->CCMR2|=(TIM_CCMR2_OC3M_2);  //force low, now!
	return 1;
}

void TIM2CC4Initialize(void)
{
//configure GPIOB10 output using TIM2 CC3 as source as step
	#define TIM2_AF 0x01
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOBEN; //enable clock to gpiob;
	GPIOB->MODER	|=	GPIO_MODER_MODE11_1;
	GPIOB->MODER	&= ~GPIO_MODER_MODE11_0;
	GPIOB->AFR[1] |= (TIM2_AF<<12);

	RCC->APB1ENR1  |= RCC_APB1ENR1_TIM2EN; //enable clock to tim2;
	TIM2->CR1     &=~ TIM_CR1_CEN;
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=(TIM_CCMR2_OC4M_2);  //force low, now!
	TIM2->CCER|=TIM_CCER_CC4E;  //enable compare channel 3
	TIM2->DIER&=~TIM_DIER_CC4IE;  //enable compare channel 3 interrupt
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CR1     |= TIM_CR1_CEN;  //enable counter

	tim2cc4.asteps=26400;  //setup defaults
	tim2cc4.pace=1024;
	tim2cc4.drift=3200;
	tim2cc4.driftPace=0x4000;

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim2_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM2_IRQn, tim2_pri_encoding );
    NVIC_EnableIRQ( TIM2_IRQn );
    return;
}

int TIM2CC4InitMove(int steps, int toState  )
{
	TIM2->DIER&=~TIM_DIER_CC4IE;  //disable compare channel 1 interrupt
	tim2cc4.toState=toState;
	tim2cc4.running=STEPRUNNING;
	tim2cc4.steps=steps;
	tim2cc4.usteps=tim2cc4.asteps;
	tim2cc4.step=1;
	tim2cc4.C = (int) isqrt(tim2cc4.usteps);
	tim2cc4.C *= tim2cc4.pace;
	steps=(tim2cc4.usteps*2)+tim2cc4.drift;
	if(steps>tim2cc4.steps) //ramp up down drift is longer than the move
	{
		tim2cc4.usteps=(tim2cc4.steps-tim2cc4.drift)/2;  //now ustep is half of the total move
		tim2cc4.dsteps=tim2cc4.usteps; //switch from astep to dstep
	}
	else tim2cc4.dsteps=tim2cc4.steps-tim2cc4.usteps-tim2cc4.drift;
	tim2cc4.start = tim2cc4.C;
	tim2cc4.finish = tim2cc4.start/2;
	tim2cc4.delta = tim2cc4.finish;
	tim2cc4.numer = 0;
	tim2cc4.odd = 3;
	tim2cc4.intsqrt=1;
	tim2cc4.intsqrtmod=0;
	tim2cc4.numer=tim2cc4.C*tim2cc4.odd;
	tim2cc4.start= tim2cc4.intsqrt*tim2cc4.odd;
    tim2cc4.P=tim2cc4.numer/tim2cc4.start;
    if(tim2cc4.drift==0) tim2cc4.driftPace = 0x7fffffff;


	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC4M_2;  //force low, now!
	tim2cc4.HL=0; //sync the indicator flag
	tim2cc4.counter = TIM2->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim2cc4.counter += tim2cc4.P;
	TIM2->CCR4 = tim2cc4.counter;  //set first transition count
	TIM2->SR &= ~(TIM_SR_CC4IF);//clear the interrupt flag
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC4M_0|TIM_CCMR2_OC4M_1;  //set to toggle mode
	TIM2->DIER|=TIM_DIER_CC4IE;  //enable compare channel 1 interrupt
	return 1;

}

int TIM2CC4status(motionstruct *currentStatus)
{

	currentStatus->counter = tim2cc4.counter;
	currentStatus->HL = tim2cc4.HL;
	currentStatus->step = tim2cc4.step;
	currentStatus->toState = tim2cc4.toState;
	currentStatus->running = tim2cc4.running;
	currentStatus->asteps=tim2cc4.asteps;
	currentStatus->drift=tim2cc4.drift;
	currentStatus->driftPace=tim2cc4.driftPace;
	currentStatus->pace=tim2cc4.pace;

	return 1;
}

int TIM2CC4set(int acceleration,int plateau, int drift,int driftPace)
{
	tim2cc4.asteps=acceleration;
	tim2cc4.pace=plateau;
	tim2cc4.drift=drift;
	tim2cc4.driftPace=driftPace;
	return 1;
}

int TIM2CC4stop(void)
{
	TIM2->DIER&=~TIM_DIER_CC4IE;  //disable compare channel 3 interrupt
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=(TIM_CCMR2_OC4M_2);  //force low, now!
	return 1;
}

/**************************************************************
void TIM2CC4Initialize(void)
{
//configure GPIOB11 to output using TIM2 CC4 as source as step
	#define IOB
 	#define BITNUMBER 11
	#define TIMNUMBER 2
	#define CCNUMBER 4

	#define TIM2_AF 0x01

#ifdef IOA
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOAEN; //enable clock to gpioa;
#endif

#ifdef IOB
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOBEN; //enable clock to gpioa;
#endif

#ifdef IOA
	GPIOA->MODER	|=	2<<((BITNUMBER*2));// GPIO_MODER_MODE11_1;
	GPIOA->MODER	&= ~(1<<((BITNUMBER*2))); //GPIO_MODER_MODE11_0;
	GPIOA->AFR[((BITNUMBER*4)/32)] |= (TIM2_AF<<((BITNUMBER*4)%32));
#endif

#ifdef IOB
	GPIOB->MODER	|=	2<<((BITNUMBER*2));// GPIO_MODER_MODE11_1;
	GPIOB->MODER	&= ~(1<<((BITNUMBER*2))); //GPIO_MODER_MODE11_0;
	GPIOB->AFR[((BITNUMBER*4)/32)] |= (TIM2_AF<<((BITNUMBER*4)%32));
#endif

	RCC->APB1ENR1  |= ((RCC_APB1ENR1_TIM2EN)<<(TIMNUMBER-2)); //enable clock to tim2;

#if (TIMNUMBER==2)
	TIM2->CR1     &=~TIM_CR1_CEN;
#if (CCNUMBER <2)
	TIM2->CCMR1&=~((TIM_CCMR1_OC1M)<<(((CCNUMBER-1)%2)*8)); //clear it out to frozen mode
	TIM2->CCMR1|=((TIM_CCMR1_OC1M_2)<<((CCNUMBER-1)%2)*8);  //force low, now!
#else
	TIM2->CCMR2&=~((TIM_CCMR1_OC1M)<<(((CCNUMBER-1)%2)*8)); //clear it out to frozen mode
	TIM2->CCMR2|=((TIM_CCMR1_OC1M_2)<<((CCNUMBER-1)%2)*8);  //force low, now!
#endif
	TIM2->CCER|=((TIM_CCER_CC1E)<<((CCNUMBER-1)*4));  //enable compare channel 1
	TIM2->DIER|=((TIM_DIER_CC1IE)<<((CCNUMBER-1)*4));  //enable compare channel 1 interrupt
#if (CCNUMBER <2)
	TIM2->CCMR1&=~((TIM_CCMR1_OC1M)<<(((CCNUMBER-1)%2)*8)); //clear it out to frozen mode
#else
	TIM2->CCMR2&=~((TIM_CCMR1_OC1M)<<(((CCNUMBER-1)%2)*8)); //clear it out to frozen mode
#endif
	TIM2->CR1     |= TIM_CR1_CEN;  //enable counter
#endif

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim2_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM2_IRQn, tim2_pri_encoding );
    NVIC_EnableIRQ( TIM2_IRQn );
    return;
}

int TIM2CC4InitMove(int step, int increment )
{
	TIM2->DIER&=~TIM_DIER_CC4IE;  //disable compare channel 1 interrupt
	tim2cc4.step=step;
	tim2cc4.increment=increment;
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC4M_2;  //force low, now!
	tim2cc4.HL=0; //sync the indicator flag
	tim2cc4.counter = TIM2->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim2cc4.counter += tim2cc4.increment;
	TIM2->CCR4 = tim2cc4.counter;  //set first transition count
	TIM2->SR &= ~(TIM_SR_CC4IF);//clear the interrupt flag
	TIM2->CCMR2&=~(TIM_CCMR2_OC4M); //clear it out to frozen mode
	TIM2->CCMR2|=TIM_CCMR2_OC4M_0|TIM_CCMR2_OC4M_1;  //set to toggle mode
	TIM2->DIER|=TIM_DIER_CC4IE;  //enable compare channel 1 interrupt
	return 1;
}

int TIM2CC4status(stepstruct *currentStatus)
{

	currentStatus->increment = tim2cc4.increment;
	currentStatus->counter = tim2cc4.counter;
	currentStatus->HL = tim2cc4.HL;
	currentStatus->step = tim2cc4.step;
	currentStatus->toState = tim2cc4.toState;
	currentStatus->running = tim2cc4.running;

	return 1;
}
*************************************************/
void TIM5CC2Initialize(void)
{
	//configure GPIOF7 to output using TIM5 CC2 as source as step
	#define TIM5_AF 0x02
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOFEN; //enable clock to gpiof;
	GPIOF->MODER	|=	GPIO_MODER_MODE7_1;
	GPIOF->MODER	&= ~GPIO_MODER_MODE7_0;
	GPIOF->AFR[0] |= (TIM5_AF<<28);

	RCC->APB1ENR1  |= RCC_APB1ENR1_TIM5EN; //enable clock to tim2;
	TIM5->CR1     &=~ TIM_CR1_CEN;
	TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
	TIM5->CCMR1|=TIM_CCMR1_OC2M_2;  //force low, now!
	TIM5->CCER|=TIM_CCER_CC2E;  //enable compare channel 1
	TIM5->DIER&=~TIM_DIER_CC2IE;  //enable compare channel 1 interrupt
	TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
	TIM5->CR1     |= TIM_CR1_CEN;  //enable counter

	tim5cc2.asteps=26400;  //setup defaults
	tim5cc2.pace=1024;
	tim5cc2.drift=3200;
	tim5cc2.driftPace=0x4000;

    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim5_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM5_IRQn, tim5_pri_encoding );
    NVIC_EnableIRQ( TIM5_IRQn );

    return;
}

int TIM5CC2InitMove(int steps, int toState )
{
	TIM5->DIER&=~TIM_DIER_CC2IE;  //disable compare channel 1 interrupt
	tim5cc2.toState=toState;
	tim5cc2.running=STEPRUNNING;
	tim5cc2.steps=steps;
	tim5cc2.usteps=tim5cc2.asteps;
	tim5cc2.step=1;
	tim5cc2.C = (int) isqrt(tim5cc2.usteps);
	tim5cc2.C *= tim5cc2.pace;
	steps=(tim5cc2.usteps*2)+tim5cc2.drift;
	if(steps>tim5cc2.steps) //ramp up down drift is longer than the move
	{
		tim5cc2.usteps=(tim5cc2.steps-tim5cc2.drift)/2;  //now ustep is half of the total move
		tim5cc2.dsteps=tim5cc2.usteps; //switch from astep to dstep
	}
	else tim5cc2.dsteps=tim5cc2.steps-tim5cc2.usteps-tim5cc2.drift;
	tim5cc2.start = tim5cc2.C;
	tim5cc2.finish = tim5cc2.start/2;
	tim5cc2.delta = tim5cc2.finish;
	tim5cc2.numer = 0;
	tim5cc2.odd = 3;
	tim5cc2.intsqrt=1;
	tim5cc2.intsqrtmod=0;
	tim5cc2.numer=tim5cc2.C*tim5cc2.odd;
	tim5cc2.start= tim5cc2.intsqrt*tim5cc2.odd;
    tim5cc2.P=tim5cc2.numer/tim5cc2.start;
    if(tim5cc2.drift==0) tim5cc2.driftPace = 0x7fffffff;

	TIM5->CCMR2&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
	TIM5->CCMR2|=TIM_CCMR1_OC2M_2;  //force low, now!
	tim5cc2.HL=0; //sync the indicator flag
	tim5cc2.counter = TIM5->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim5cc2.counter += tim5cc2.P;
	TIM5->CCR2 = tim5cc2.counter;  //set first transition count
	TIM5->SR &= ~(TIM_SR_CC2IF);//clear the interrupt flag
	TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
	TIM5->CCMR1|=TIM_CCMR1_OC2M_0|TIM_CCMR1_OC2M_1;  //set to toggle mode
	TIM5->DIER|=TIM_DIER_CC2IE;  //enable compare channel 1 interrupt
	return 1;
}

int TIM5CC2status(motionstruct *currentStatus)
{

	currentStatus->counter = tim5cc2.counter;
	currentStatus->HL = tim5cc2.HL;
	currentStatus->step = tim5cc2.step;
	currentStatus->toState = tim5cc2.toState;
	currentStatus->running = tim5cc2.running;
	currentStatus->asteps=tim5cc2.asteps;
	currentStatus->drift=tim5cc2.drift;
	currentStatus->driftPace=tim5cc2.driftPace;
	currentStatus->pace=tim5cc2.pace;

	return 1;
}

int TIM5CC2set(int acceleration,int plateau, int drift,int driftPace)
{
	tim5cc2.asteps=acceleration;
	tim5cc2.pace=plateau;
	tim5cc2.drift=drift;
	tim5cc2.driftPace=driftPace;
	return 1;
}

int TIM5CC2stop(void)
{
	TIM5->DIER&=~TIM_DIER_CC2IE;  //disable compare channel 3 interrupt
	TIM5->CCMR1&=~(TIM_CCMR1_OC2M); //clear it out to frozen mode
	TIM5->CCMR1|=(TIM_CCMR1_OC2M_2);  //force low, now!
	return 1;
}

void TIM5CC3Initialize(void)
{
	//configure GPIOF8 to output using TIM5 CC3 as source as step
	#define TIM5_AF 0x02
	RCC->AHB2ENR	|=	RCC_AHB2ENR_GPIOFEN; //enable clock to gpioa;
	GPIOF->MODER	|=	GPIO_MODER_MODE8_1;
	GPIOF->MODER	&= ~GPIO_MODER_MODE8_0;
	GPIOF->AFR[1] |= (TIM5_AF<<0);

	RCC->APB1ENR1  |= RCC_APB1ENR1_TIM5EN; //enable clock to tim2;
	TIM5->CR1     &=~ TIM_CR1_CEN;
	TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM5->CCMR2|=TIM_CCMR2_OC3M_2;  //force low, now!
	TIM5->CCER|=TIM_CCER_CC3E;  //enable compare channel 1
	TIM5->DIER&=~TIM_DIER_CC3IE;  //enable compare channel 1 interrupt
	TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM5->CR1     |= TIM_CR1_CEN;  //enable counter

	tim5cc3.asteps=26400;  //setup defaults
	tim5cc3.pace=1024;
	tim5cc3.drift=3200;
	tim5cc3.driftPace=0x4000;


    // Setup the NVIC to enable interrupts.
    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // TIM interrupts should be high priority.
    uint32_t tim2_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( TIM5_IRQn, tim2_pri_encoding );
    NVIC_EnableIRQ( TIM5_IRQn );
    return;
}

int TIM5CC3InitMove(int steps, int toState )
{
	TIM5->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 1 interrupt
	tim5cc3.toState=toState;
	tim5cc3.running=STEPRUNNING;
	tim5cc3.steps=steps;
	tim5cc3.usteps=tim5cc3.asteps;
	tim5cc3.step=1;
	tim5cc3.C = (int) isqrt(tim5cc3.usteps);
	tim5cc3.C *= tim5cc3.pace;
	steps=(tim5cc3.usteps*2)+tim5cc3.drift;
	if(steps>tim5cc3.steps) //ramp up down drift is longer than the move
	{
		tim5cc3.usteps=(tim5cc3.steps-tim5cc3.drift)/2;  //now ustep is half of the total move
		tim5cc3.dsteps=tim5cc3.usteps; //switch from astep to dstep
	}
	else tim5cc3.dsteps=tim5cc3.steps-tim5cc3.usteps-tim5cc3.drift;
	tim5cc3.start = tim5cc3.C;
	tim5cc3.finish = tim5cc3.start/2;
	tim5cc3.delta = tim5cc3.finish;
	tim5cc3.numer = 0;
	tim5cc3.odd = 3;
	tim5cc3.intsqrt=1;
	tim5cc3.intsqrtmod=0;
	tim5cc3.numer=tim5cc3.C*tim5cc3.odd;
	tim5cc3.start= tim5cc3.intsqrt*tim5cc3.odd;
    tim5cc3.P=tim5cc3.numer/tim5cc3.start;
    if(tim5cc3.drift==0) tim5cc3.driftPace = 0x7fffffff;

	TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM5->CCMR2|=TIM_CCMR2_OC3M_2;  //force low, now!
	tim5cc3.HL=0; //sync the indicator flag
	tim5cc3.counter = TIM5->CNT;  //not sure if it was high or low so first time will be pace long low, initialize counter
	tim5cc3.counter += tim5cc3.P;
	TIM5->CCR3 = tim5cc3.counter;  //set first transition count
	TIM5->SR &= ~(TIM_SR_CC3IF);//clear the interrupt flag
	TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM5->CCMR2|=TIM_CCMR2_OC3M_0|TIM_CCMR2_OC3M_1;  //set to toggle mode
	TIM5->DIER|=TIM_DIER_CC3IE;  //enable compare channel 1 interrupt
	return 1;
}

int TIM5CC3status(motionstruct *currentStatus)
{

	currentStatus->counter = tim5cc3.counter;
	currentStatus->HL = tim5cc3.HL;
	currentStatus->step = tim5cc3.step;
	currentStatus->toState = tim5cc3.toState;
	currentStatus->running = tim5cc3.running;
	currentStatus->asteps=tim5cc3.asteps;
	currentStatus->drift=tim5cc3.drift;
	currentStatus->driftPace=tim5cc3.driftPace;
	currentStatus->pace=tim5cc3.pace;

	return 1;
}

int TIM5CC3set(int acceleration,int plateau, int drift,int driftPace)
{
	tim5cc3.asteps=acceleration;
	tim5cc3.pace=plateau;
	tim5cc3.drift=drift;
	tim5cc3.driftPace=driftPace;
	return 1;
}

int TIM5CC3stop(void)
{
	TIM5->DIER&=~TIM_DIER_CC3IE;  //disable compare channel 3 interrupt
	TIM5->CCMR2&=~(TIM_CCMR2_OC3M); //clear it out to frozen mode
	TIM5->CCMR2|=(TIM_CCMR2_OC3M_2);  //force low, now!
	return 1;
}
