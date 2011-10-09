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
static float Station[3]; //DECL::VAR::Station
static float Asteroid[3]; //DECL::VAR::Asteroid
static char switchedStation; //DECL::VAR::switchedStation
static char op; //DECL::VAR::op
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
#define thetastep   dist
#define theta       mag
#define dirs        Laser

float opulens[3] = {0.0, -0.6, 0.0};
float Laser[3] = {0.4, 0.0, 0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3], target[3], temp[3];
float dist, mag;



if(!time)
{
    SphereNumber = !!(myState[0] < 0) + 1;
    Station[0] = (SphereNumber == 1) ? 0.6 : -0.6;
}

Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;

if((time > 130) && (switchedStation == 0 || switchedStation > 2) && !(state > 1)){
    mathVecSubtract(difference, Station, otherState, 3);
    temp[0] = mathVecInner(&otherState[3], difference, 3) / (mathVecMagnitude(&otherState[3], 3) * mathVecMagnitude(difference, 3));
    Station[1] *= -1;
    mathVecSubtract(difference, Station, otherState, 3);
    temp[1] = mathVecInner(&otherState[3], difference, 3) / (mathVecMagnitude(&otherState[3], 3)* mathVecMagnitude(difference, 3));
    if(temp[1] < temp[0])
        Station[1] *= -1;
}


switch((int)PgetMessage())
{
    case 6:
        if(!(Station[0] < 0.0) && switchedStation++ < 3)
            Station[0] = -0.6;
        break;
    case 7:
        if(!(Station[0] > 0.0) && switchedStation++ < 3)
            Station[0] = 0.6;
        break;
    case 2:
    case 3:
        if(time < 40)
            op = 1;
        break;
    case 4:
    case 5:
        if(time < 40)
            op = 0;
        break;
    default:
        if(((int)PgetMessage() & 0x0400) && (time < 40)) // y0b0tics: both of us are fully capable, so SPH1 orbits, SPH2 spins
        {
            op = 1;
            action = SphereNumber % 2;
        }
        else if(((int)PgetMessage() & 0x0200) && (time < 40)) // y0b0tics: they're not as capable, so we orbit
           // not specific enough, delta mk2 should tell us exactly what they can/cannot do
        {
            op = 1;
            action = 1;
        }
        break;
}

PsendMessage(0x0400); // y0b0tics: "I can do everything"

Asteroid[1] = (op) ? -0.6 : 0.6;

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
                action = 0;
            }
            switchTimer = 10;
            cooloff = 8;
        }
        if(switchTimer == 15)
        {
            action = 1;
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
                if(PisRevolving(otherState) == 2 - op)
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
                if(mathVecMagnitude(difference, 3) > .6)
                {
                    ZRSetPositionTarget(Asteroid);
                }
                else
                {
                    
                    ZRSetVelocityTarget(zero);
                }
                shoot(myState, opulens, (PgetPhase() > 1));
                return;
            }
            
            theta = atan2f(difference[1],difference[0]);
            
            thetastep = -0.66;
            
            theta += thetastep;
            difference[0] = .4 * sinf(PI/2 - theta);
            difference[1] = .4 * sinf(theta);
            mathVecAdd(target, Asteroid, difference, 3);
            ZRSetPositionTarget(target);
            
        }
        else
        {
            if(!cooloff)
            {
                mathVecSubtract(temp, otherState, Asteroid, 3);
                if((PinAsteroid(otherState) == 2 - op) || ((mathVecMagnitude(temp, 3) < .2) && (fabsf(otherState[11]) > PI/12)))
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
                ZRSetPositionTarget(opulens);
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
        
        if(((((mathVecMagnitude(difference, 3) / 0.07) + 
            (acos(mathVecInner(&myState[3], difference, 3)/(mathVecMagnitude(&myState[3],3)*mathVecMagnitude(difference,3)))*10/PI) +
            ((0.08 - mathVecMagnitude(&myState[3], 3)) / .01) +
            7.0) + time >= 168.0) && action)
            || (time > 153 && !action))
            state++;
            
        break;
    
    default:
    
        mathVecSubtract(difference, Asteroid, myState, 3);
        mathVecSubtract(target, Station, myState, 3);

        dist = mathVecMagnitude(target, 3);

        if(dist < .19)
            mag = (mathVecMagnitude(&myState[3], 3) > 0.01) ? 0.002 : 0.01;
        else if(dist < 0.33)
            mag = dist/14;
        else{
            mathVecCross(temp, difference, &myState[3]);
            mag = mathVecMagnitude(difference, 3)*3.141592/(45*mathVecMagnitude(temp, 3));
        }
    
        mag = (mag > dist/8.5) ? dist/8.5 : mag;
        mag = (mag > 0.08) ? 0.08 : mag;

        mathVecNormalize(target, 3);

        target[0] *= mag;
        target[1] *= mag;

        ZRSetVelocityTarget(target);

        break;
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
memset(Station,0,sizeof(float)*3);
memset(Asteroid,0,sizeof(float)*3);
switchedStation = 0;
op = 1;
//END::PROC::ZRInit
}
//User-defined procedures
static void shoot (float myState[12], float target[3], unsigned int fire)
{
//BEGIN::PROC::shoot
float direction[3];

mathVecSubtract(direction, target, myState, 3);
mathVecNormalize(direction, 3);

ZRSetAttitudeTarget(direction);

if (fire && (mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) > (0.995))
    Plaser();
//END::PROC::shoot
}
