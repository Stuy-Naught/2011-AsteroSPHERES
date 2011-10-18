#include <math.h>
#include <string.h>
#include "ZR_API.h"
#include "ZRGame.h"
#include "math_matrix.h"
#include "spheres_constants.h"

#ifdef ZRSIMULATION
extern void _init(void);
void *_start = &_init;
#endif

static unsigned char state; //DECL::VAR::state
static unsigned char SphereNumber; //DECL::VAR::SphereNumber
static char action; //DECL::VAR::action
static char switchTimer; //DECL::VAR::switchTimer
static char cooloff; //DECL::VAR::cooloff
static float Asteroid[3]; //DECL::VAR::Asteroid
static char op; //DECL::VAR::op
static char y0received; //DECL::VAR::y0received
static int lastMessage; //DECL::VAR::lastMessage
static int gotToTangent; //DECL::VAR::gotToTangent
static float Station[3]; //DECL::VAR::Station
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
#define thetastep   dist
#define theta       mag
#define dirs        Laser

  float opulens[3] = {0.0, -0.6, 0.0};
  float opulens_spin_point[3] = {0.04, -0.6, 0.0};
  float Laser[3] = {0.4, 0.0, 0.0};
  float difference[3], target[3];
  float dist, mag, tandist, xangle;
  
  if(!time)
    {
      SphereNumber = !!(myState[0] < 0) + 1;
    }

  Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;
  
  if((int)PgetMessage())
      lastMessage = (int)PgetMessage();
    
  switch(lastMessage)
    {
    case 2:
    case 3:
      if((time < 20) && !y0received && (time))
	op = 1;
      break;
    case 4:
    case 5:
      if((time < 20) && !y0received && (time))
	op = 0;
      break;
    default:
      if(((int)PgetMessage() & 0x0400) && (time < 40)) // y0b0tics: both of us are fully capable, so SPH1 orbits, SPH2 spins
	{
	  op = 1;
	  action = SphereNumber % 2;
      y0received = 1;
	}
      else if(((int)PgetMessage() & 0x0200) && (time < 40)) // y0b0tics: they're not as capable, so we orbit
	// not specific enough, delta mk2 should tell us exactly what they can/cannot do
	{
	  op = 1;
	  action = 1;
      y0received = 1;
	}
      break;
    }
    
  if(!time || y0received)
      PsendMessage(0x0400); // y0b0tics: "I can do everything"
  else if(time == 1)
      PsendMessage((unsigned short)(3)); // I will orbit on Opulens
  else
      PsendMessage((unsigned short)(4 - (2 * op) + action));
    
  Asteroid[1] = (op) ? -0.6 : 0.6;
		      
if(!state){
      ZRSetPositionTarget(Laser);
      shoot(myState, opulens, 0);
      if(PhaveLaser() || PgetPhase() == 2)
	{
	  state = 1;
	}
}
else{
    if(switchTimer == 5)
	{
	  if(time < 130)
	    {
	      action = 0;
	    }
	  switchTimer = 10;
	  cooloff = 8;
	}
      if(switchTimer == 15)
	{
      if(time < 130)
        {
	      action = 1;
        }
	  switchTimer = 0;
	  cooloff = 8;
	}
			      
      if(cooloff)
	    {
	    cooloff--;
	    }
				
      if(action)
	    {
	  if(!cooloff)
	    {
          mathVecSubtract(difference, Asteroid, otherState, 3);
          mathVecNormalize(difference, 3);
	      if((PisRevolving(otherState) == 2 - op) && (mathVecInner(&otherState[3], difference, 3) < (0.9 * mathVecMagnitude(&otherState[3], 3))))
		{
		  if(switchTimer < 5 && switchTimer >= 0)
		    {
		      switchTimer++;
		    }
		  else
		    {
		      switchTimer = 1;
		    }
		}
	      else
		{
		  switchTimer = 0;
		}
	    }
				      
	  mathVecSubtract(difference, myState, Asteroid, 3);
					
	  if((!PiceMelted()) && op)
	    {
	      shoot(myState, opulens, (PgetPhase() > 1));
	    }
					  


	  if (mathVecMagnitude(difference, 3) < (0.5 + gotToTangent/10))
        {
        gotToTangent = 1;
	    theta = atan2f(difference[1],difference[0]);
	    thetastep = 0.070996444 * ((SphereNumber * 2) - 3); // TAU / 88.5, zach's magic value
	    theta += thetastep;
	    difference[0] = .45 * sinf(PI/2 - theta);
	    difference[1] = .45 * sinf(theta);
	    mathVecAdd(target, Asteroid, difference, 3); // target is new position 
	    mathVecSubtract(difference, target, myState, 3); // difference points from myState to target 
	    ZRSetVelocityTarget(difference); 
	    }
      else
        {
        mathVecSubtract(difference, myState, Asteroid, 3);
        dist = mathVecMagnitude(difference, 3);
        
        tandist = sqrt(dist*dist - 0.2025);
        theta = acos(tandist/dist);
        
        xangle = atan2f(Asteroid[1] - myState[1], Asteroid[0] - myState[0]);

        theta *= ((SphereNumber * (-2)) + 3);
        
        target[0] = myState[0] + (tandist * sinf(PI/2 - theta - xangle));
        target[1] = myState[1] + (tandist * sinf(theta + xangle));
        target[2] = 0;
        ZRSetPositionTarget(target);
        }
	}
      else
	{
	  if(!cooloff)
	    {
	      mathVecSubtract(difference, otherState, Asteroid, 3);
	      if(!(PisRevolving(otherState) == 2 - op))
		{
		  if(switchTimer < 15 && switchTimer >= 10)
		    {
		      switchTimer++;
		    }
		  else
		    {
		      switchTimer = 11;
		    }
		}
	      else
		{
		  switchTimer = 10;
		}
	    }

	  if(!(PiceMelted()) && op)
	    {
	      opulens_spin_point[0] = (SphereNumber == 1) ? 0.04 : -0.04;
	      ZRSetPositionTarget(opulens_spin_point);
	      shoot(myState, opulens, (PgetPhase() > 1));
	      return;
	    }

	  dirs[2] = 1.872459067 + atan2f(myState[7], myState[6]);
	  dirs[0] = sinf(PI/2 - dirs[2]);
	  dirs[1] = sinf(dirs[2]);
	  dirs[2] = 0;
						
	  ZRSetAttitudeTarget(dirs);
						  
	  ZRSetPositionTarget(Asteroid);
      
	}
}
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
state = 0;
SphereNumber = 0;
action = 1;
switchTimer = 0;
cooloff = 0;
memset(Asteroid,0,sizeof(float)*3);
op = 1;
y0received = 0;
lastMessage = 0;
gotToTangent = 0;
memset(Station,0,sizeof(float)*3);
//END::PROC::ZRInit
}
//User-defined procedures
static void shoot (float myState[12], float target[3], unsigned int fire)
{
//BEGIN::PROC::shoot
float direction[3];
float new_pos[3];

// new_pos:  predict my new position in 2 seconds
mathVecAdd(new_pos, &myState[3], &myState[3], 3); 
mathVecAdd(new_pos, myState, new_pos, 3);

// direction: where should I aim in the future
mathVecSubtract(direction, target, new_pos, 3);
mathVecNormalize(direction, 3);
      
ZRSetAttitudeTarget(direction);

// direction: where should I be pointing NOW
mathVecSubtract(direction, target, myState, 3);
mathVecNormalize(direction, 3);

// shoot if I "see" the target
if (fire && (mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) > (0.995))
  Plaser();

//END::PROC::shoot
}
