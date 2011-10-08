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
static char shouldOrbit; //DECL::VAR::action
static char switchTimer; //DECL::VAR::switchTimer
static char cooloff; //DECL::VAR::cooloff
static void orbit (float myState[12], float center[3], unsigned char CCW); //DECL::PROC::orbit
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot
static void spin (float myState[12]); //DECL::PROC::spin

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
float opulens[3] = {0.0, -0.6, 0.0};
float Laser[3] = {0.4, 0.0, 0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3];
float Asteroid[3] = {0.0, -0.6, 0.0};
float Station[3] = {0.0, 0.0, 0.0};


if(!time)
{
    SphereNumber = !!(myState[0] < 0) + 1;
}

Station[0] = (SphereNumber == 1) ? 0.6 : -0.6;
Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;


  // Following y0b0tics mining guild protocol
  PsendMessage(1 << 10); // we can do everything (bit 11 turned on)

  if((int)PgetMessage() & 0x0400) // they can do everything (bit 11 turned on)
    {
      shouldOrbit = SphereNumber % 2; // SPH1 orbits, SPH2 spins
    }
  else if((int)PgetMessage() & 0x0200) // they can't do something
    {
      shouldOrbit = 1; // we get to orbit!
    }
  

switch(state)
{
    case 0:
        ZRSetPositionTarget(Laser);
        shoot(myState, opulens, 0);
        if(PhaveLaser() || PgetPhase() == 2)
        {
            state = 1;
        }
        break;
        
    case 1:
        if(switchTimer == 5)
        {
            if(time > 120)
            {
                shouldOrbit = 0;
            }
            switchTimer = 10;
            cooloff = 8;
        }
        if(switchTimer == 15)
        {
            shouldOrbit = 1;
            switchTimer = 0;
            cooloff = 8;
        }
        
        if(cooloff)
        {
            cooloff--;
        }
        
        if(orbit)
        {
            if(!cooloff)
            {
                if(PisRevolving(otherState) == 1)
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
            
            if(!(PiceMelted()))
            {
                mathVecSubtract(difference, myState, opulens, 3);
                if(mathVecMagnitude(difference, 3) > .6)
                {
                    ZRSetPositionTarget(opulens);
                }
                else
                {
                    ZRSetVelocityTarget(zero);
                }
                shoot(myState, opulens, (PgetPhase() > 1));
                return;
            }
            orbit(myState, Asteroid, 1);
        }
        else
        {
            if(!cooloff)
            {
                if(PinAsteroid(otherState) == 1)
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
            
            if(!(PiceMelted()))
            {
                ZRSetPositionTarget(opulens);
                shoot(myState, opulens, (PgetPhase() > 1));
                return;
            }
            spin(myState);
            ZRSetPositionTarget(opulens);
        }
        break;
    
    default:
        ZRSetPositionTarget(Station);
        break;
}
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
state = 0;
SphereNumber = 0;
shouldOrbit = 1;
switchTimer = 0;
cooloff = 0;
//END::PROC::ZRInit
}
//User-defined procedures
static void orbit (float myState[12], float center[3], unsigned char CCW)
{
//BEGIN::PROC::orbit
float difference[3], target[3];
float theta;
float thetastep = .66;

mathVecSubtract(difference, myState, center, 3);
theta = atan2f(difference[1],difference[0]);

if(CCW)
    thetastep *= -1;

theta += thetastep;
difference[0] = .4 * sinf(PI/2 - theta);
difference[1] = .4 * sinf(theta);
mathVecAdd(target, center, difference, 3);
ZRSetPositionTarget(target);
//END::PROC::orbit
}
static void shoot (float myState[12], float target[3], unsigned int fire)
{
//BEGIN::PROC::shoot
float direction[3];

mathVecSubtract(direction, target, myState, 3);
mathVecNormalize(direction, 3);

ZRSetAttitudeTarget(direction);

if (fire && (fabsf(mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) > (0.995)))
    Plaser();
//END::PROC::shoot
}
static void spin (float myState[12])
{
//BEGIN::PROC::spin
float dirs[3];
dirs[2] = 1.872459067 + atan2f(myState[7], myState[6]);
dirs[0] = sinf(PI/2 - dirs[2]);
dirs[1] = sinf(dirs[2]);
dirs[2] = 0;

ZRSetAttitudeTarget(dirs);
//END::PROC::spin
}
