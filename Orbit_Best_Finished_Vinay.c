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
static char getShield; //DECL::VAR::getShield
static char asteroid; //DECL::VAR::asteroid
static void orbit (float myState[12], float center[3], unsigned char CCW); //DECL::PROC::orbit
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot
static float timeToMS (float myState[12], float station[3]); //DECL::PROC::timeToMS

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
float opulens[3] = {0.0, -0.6, 0.0};
float Laser[3] = {0.4, 0.0, 0.0};
float shield[3] = {0.0, 0.4, 0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3];
float Asteroid[3] = {0.0, -0.6, 0.0};
float Station[3] = {0.0, 0.0, 0.0};


switch((int)time)
{
    case 0:
        SphereNumber = !!(myState[0] < 0) + 1;
        break;
    case 30:
        getShield = (otherState[1] <= 0);
        break;
    case 61:
        asteroid = (PinAsteroid(otherState) == 1) ? 1 : 0;
        break;
    case 90:
        asteroid = (PisRevolving(otherState) == 2 && !asteroid) ? 1 : asteroid;
        break;
}


Station[0] = (SphereNumber == 1) ? 0.6 : -0.6; //station closer to where you started
Asteroid[1] = asteroid ? -0.6 : 0.6;
Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;


switch(state)
{
    case 0:
        ZRSetPositionTarget(Laser);
        shoot(myState, opulens, 0);
        if(PhaveLaser() || PgetPhase() == 2)
            state = 1;
        break;
        
    case 1:
        if(!getShield)
            shield[1] = .3;
        ZRSetPositionTarget(shield);
        shoot(myState, opulens, 0);
        if(PotherHasShield() || PhaveShield() || PgetPhase() == 2)
            state = 2;
        break;
        
    case 2:
        if(PiceHits() < 14 && !asteroid)
            shoot(myState, opulens, (PgetPhase() > 1));
        if(!(PiceMelted()) && asteroid)
        {
            mathVecSubtract(difference, myState, opulens, 3);
            if(mathVecMagnitude(difference, 3) > .8)
                ZRSetPositionTarget(opulens);
            else
                ZRSetVelocityTarget(zero);
            shoot(myState, opulens, (PgetPhase() > 1));
            return;
        }
        orbit(myState, Asteroid, Station[0] == -0.6);
        if(time + timeToMS(myState, Station) >= 165)
            state = 3;
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
getShield = 1;
asteroid = 0;
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
theta = atan2(difference[1], difference[0]);

if(CCW)
    thetastep *= -1;

theta += thetastep;
difference[0] = .4 * cos(theta);
difference[1] = .4 * sin(theta);
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

if (fire && (acos(mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) < (0.1)))
    Plaser();
//END::PROC::shoot
}
static float timeToMS (float myState[12], float station[3])
{
//BEGIN::PROC::timeToMS
// distance / average velocity
// + time to turn around
float toStation[3];

//this is ugly so that code can fit
mathVecSubtract(toStation, station, myState, 3);
return (mathVecMagnitude(toStation, 3) / 0.065) + 
(acos(mathVecInner(&myState[3], toStation, 3)/(mathVecMagnitude(&myState[3],3)*mathVecMagnitude(toStation,3)))/18) +
((0.06 - mathVecMagnitude(&myState[3], 3)) / .01) +
8;
//END::PROC::timeToMS
}
