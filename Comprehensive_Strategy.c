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
static char asteroid; //DECL::VAR::asteroid
static char Spin; //DECL::VAR::Spin
static int switchedStation; //DECL::VAR::switchedStation
static float Station[3]; //DECL::VAR::Station
static int getUp; //DECL::VAR::getUp
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot
static float timeToMS (float myState[12], float station[3]); //DECL::PROC::timeToMS
static void leaveOrbit (float myState[12], float asteroidLoc[3], float mPanel[3]); //DECL::PROC::leaveOrbit
static void orbit (float myState[12], float center[3], unsigned char CCW); //DECL::PROC::orbit
static void spin (float myState[12]); //DECL::PROC::spin

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
float opulens[3] = {0.0, -0.6, 0.0};
float Laser[3] = {0.4, 0.0, 0.0};
float shield[3] = {0.0, 0.4, 0.0};
float disrupt[3] = {0.0,0.2,0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3];
float facing[3];
float Asteroid[3] = {0.0, -0.6, 0.0};
int recieved;


switch((int)time)
{
    case 0:
        SphereNumber = !!(myState[0] < 0) + 1;
        Station[0] = (SphereNumber == 1) ? 0.6 : -0.6; //station closer to where you started
        break;
    case 30:
        getUp = (otherState[1] <= 0);
        break;
    case 61:
    case 66:
        Spin = PisRevolving(otherState) ? 1 : 0;
        break;
    case 75: if(PinAsteroid(otherState) == 1) asteroid = 1;
}


Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;

recieved = (int)PgetMessage();
switch(recieved)
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
        if(time < 45)
        asteroid = 1;
        break;
    case 4:
    case 5:
        if(time < 45)
        asteroid = 0;
        break;
}


if(time <= 120)
PsendMessage(4 - 2*asteroid + (Spin == 0));
else
PsendMessage((Station[0] < 0) + 6);

Asteroid[1] = asteroid ? -0.6 : 0.6;


switch(state)
{
    case 0:
        ZRSetPositionTarget(Laser);
        shoot(myState, opulens, 0);
        if(PhaveLaser() || PgetPhase() == 2)
            state = 1;
        break;
        
    case 1:
        if(!getUp) {
            shield[1] = .3;
            disrupt[1] = .3;
        }
        ZRSetPositionTarget(disrupt);
        shoot(myState, opulens, 0);
        if(PotherDisruptorUpgraded() || PdisruptorUpgraded())
            state = 2;
        if(PgetPhase() == 2)
            state = 3;
        break;
        
    case 2:
        ZRSetPositionTarget(shield);
        shoot(myState, opulens, 0);
        if(PotherHasShield() || PhaveShield() || PgetPhase() == 2)
            state = 3;
        break;
    case 3:
        if(PiceHits() < 15 && !asteroid)
            shoot(myState, opulens, (PgetPhase() > 1));
        if(!(PiceMelted()) && asteroid)
        {
            mathVecSubtract(difference, myState, opulens, 3);
            if(mathVecMagnitude(difference, 3) > .8)
                ZRSetPositionTarget(opulens);
            else if(!Spin)
                ZRSetVelocityTarget(zero);
            shoot(myState, opulens, (PgetPhase() > 1));
            return;
        }
        if(Spin){
            ZRSetPositionTarget(Asteroid);
            spin(myState);
        }
        else
            orbit(myState, Asteroid, Station[0] == -0.6);
        if((!Spin && (time + timeToMS(myState, Station) >= 165)) || (Spin && (time >= 156)))
            state = 4;
        break;
    default:
        leaveOrbit(myState, Asteroid, Station);
        mathVecSubtract(facing, otherState, myState, 3);
        mathVecNormalize(facing, 3);
        ZRSetAttitudeTarget(facing);
        if (acos(mathVecInner(&myState[6], facing, 3) / mathVecMagnitude(&myState[6], 3)) < (0.1))
            Ptractor();
        break;
}
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
state = 0;
SphereNumber = 0;
asteroid = 0;
Spin = 0;
switchedStation = 0;
memset(Station,0,sizeof(float)*3);
getUp = 0;
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
static void leaveOrbit (float myState[12], float asteroidLoc[3], float mPanel[3])
{
//BEGIN::PROC::leaveOrbit
#define SinAngle(a, b) sqrt(1 - mathSquare(mathVecInner((a), (b), 3)/(mathVecMagnitude((a), 3) * mathVecMagnitude((b), 3)))) 

float targetVel[3];
float r, toMS;
float desiredMag;
float vecToAst[3];
float vecToMS[3];
int i;

for(i = 0; i < 3; i++){
    vecToAst[i] = asteroidLoc[i] - myState[i];
    vecToMS[i] = mPanel[i] - myState[i];
}
toMS = mathVecMagnitude(vecToMS, 3);

r = sqrt(mathVecInner(vecToAst, vecToAst, 3));

for(i = 0; i < 3; i++)
targetVel[i] = mPanel[i] - myState[i];

if(toMS < .19){
    if(mathVecMagnitude(&myState[3], 3) > 0.01)
        desiredMag = 0.002;
    else
        desiredMag = 0.01;
}
else if(toMS < 0.25)
    desiredMag = toMS/12;
else if(toMS < 0.35)
    desiredMag = toMS/7;
else
    desiredMag = r*3.141592/(45*SinAngle(vecToAst, &myState[3]));
    
if(desiredMag > toMS/6)
    desiredMag = toMS/6;

mathVecNormalize(targetVel, 3);

for(i = 0; i < 3; i++)
    targetVel[i] *= desiredMag;
ZRSetVelocityTarget(targetVel);
//ZRSetAttitudeTarget(att);
//END::PROC::leaveOrbit
}
static void orbit (float myState[12], float center[3], unsigned char CCW)
{
//BEGIN::PROC::orbit
float difference[3], target[3];
float theta;
float thetastep = .66;

mathVecSubtract(difference, myState, center, 3);
theta = atan2f(difference[1], difference[0]);

if(CCW)
    thetastep *= -1;

theta += thetastep;
difference[0] = .4 * sinf(PI/2 - theta);
difference[1] = .4 * sinf(theta);
mathVecAdd(target, center, difference, 3);
ZRSetPositionTarget(target);
//END::PROC::orbit
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
