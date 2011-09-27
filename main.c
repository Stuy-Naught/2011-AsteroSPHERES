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

static float TAU; //DECL::VAR::TAU
static float Points; //DECL::VAR::Points
static void leaveOrbit (float myState[12], float asteroid[3], float mPanel[3]); //DECL::PROC::leaveOrbit
static void spin (float myState[12]); //DECL::PROC::spin
static void OrbitDirectionally (float center[3], float radius, float myState[3], int counterclockwise); //DECL::PROC::OrbitDirectionally
static char readyToLeaveOrbit (float myState[12], float asteroid[3], float mBase[3]); //DECL::PROC::readyToLeaveOrbit
static char TangentFinder (float myState[12], float center[3], float radius, float tangentPoint[3]); //DECL::PROC::TangentFinder
static void doStrategy (char asteroidIsIndigens, char actionIsSpin, char stationIs1, float time, float myState[12]); //DECL::PROC::doStrategy

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
doStrategy(0, 0, 0, time, myState);




//000   opulens, orbit, station2
//001   opulens, orbit, station1    
//010   opulens, spin, station2
//011   opulens, spin, station1
//100   indigens, orbit, station2
//101   indigens, orbit, station1
//110   indigens, spin, station2
//111   indigens, spin, station1
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
TAU = 6.283185307179586;
Points = 0;
//END::PROC::ZRInit
}
//User-defined procedures
static void leaveOrbit (float myState[12], float asteroid[3], float mPanel[3])
{
//BEGIN::PROC::leaveOrbit
#define Angle(v1, v2) acos(mathVecInner((v1), (v2), 3)/(mathVecMagnitude((v1), 3) * mathVecMagnitude((v2), 3)))
float targetVel[3];
float r, t;
float desiredMag;
float vecToAst[3];
int i;

for(i = 0; i < 3; i++)
vecToAst[i] = asteroid[i] - myState[i];


r = sqrt(mathVecInner(vecToAst, vecToAst, 3));

if(r > .6)
ZRSetPositionTarget(mPanel);
else{
    for(i = 0; i < 3; i++)
    targetVel[i] = mPanel[i] - myState[i];
    desiredMag = r*3.141592/(45*sin(Angle(vecToAst, targetVel)));
    for(i = 0; i < 3; i++)
    targetVel[i] *= sqrt(desiredMag);    
    ZRSetVelocityTarget(targetVel);
}
//END::PROC::leaveOrbit
}
static void spin (float myState[12])
{
//BEGIN::PROC::spin
float dirs[3];
dirs[2] = 1.872459067 + atan2(myState[7], myState[6]);
dirs[0] = cos(dirs[2]);
dirs[1] = sin(dirs[2]);
dirs[2] = 0;

ZRSetAttitudeTarget(dirs);
//END::PROC::spin
}
static void OrbitDirectionally (float center[3], float radius, float myState[3], int counterclockwise)
{
//BEGIN::PROC::OrbitDirectionally
float desiredAngularVelocity;
float velTarget[3];
float angleTarget;
float posTarget[3];
float currentAngle;
float angleStep;

currentAngle = atan2(myState[1] - center[1], myState[0] - center[0]);

//tweak angleStep as neccesary
angleStep = TAU * radius / 2;

//If counterclockwise is 1, don't change anything, because then this
//method functions exactly as Blake's original one does. Otherwise,
//it should be 0, so we multiply angleStep by -1.
angleStep *= ((2 * counterclockwise) - 1);

desiredAngularVelocity = TAU / 90;

angleTarget = currentAngle + angleStep;

posTarget[0] = radius * cos(angleTarget) + center[0];
posTarget[1] = radius * sin(angleTarget) + center[1];

velTarget[0] = posTarget[0] - myState[0];
velTarget[1] = posTarget[1] - myState[1];
velTarget[2] = posTarget[2] - myState[2];
mathVecNormalize(velTarget, 3);
velTarget[0] *= radius * desiredAngularVelocity;
velTarget[1] *= radius * desiredAngularVelocity;
velTarget[2] *= radius * desiredAngularVelocity;

ZRSetVelocityTarget(velTarget);
//END::PROC::OrbitDirectionally
}
static char readyToLeaveOrbit (float myState[12], float asteroid[3], float mBase[3])
{
//BEGIN::PROC::readyToLeaveOrbit
#define Angle(v1, v2) acos(mathVecInner((v1), (v2), 3)/(mathVecMagnitude((v1), 3) * mathVecMagnitude((v2), 3)))

float myPos[3];
float myVel[3];
float toMBase[3];
float r;
float hypotenuse;
int i;

for(i = 0; i < 3; i++)
    myPos[i] = myState[i];
    
for(; i < 6; i++)
    myVel[i - 3] = myState[i];
    
r = sqrt(mathSquare(asteroid[0] - myPos[0])
       + mathSquare(asteroid[1] - myPos[1])
       + mathSquare(asteroid[2] - myPos[2]));

hypotenuse = sqrt(mathSquare(asteroid[0] - mBase[0])
                + mathSquare(asteroid[1] - mBase[1])
                + mathSquare(asteroid[2] - mBase[2]));
    
mathVecSubtract(toMBase, mBase, myPos, 3);

//If (going in right direction) and (current distance to the mining base == length of tangent line to m base)
if((mathVecInner(myVel, toMBase, 3) > 0) &&
   (fabsf(mathVecMagnitude(toMBase, 3) - sqrt(hypotenuse*hypotenuse - r*r)) < 0.01))
    return 'y';
else
    return 'n';
//END::PROC::readyToLeaveOrbit
}
static char TangentFinder (float myState[12], float center[3], float radius, float tangentPoint[3])
{
//BEGIN::PROC::TangentFinder
float dist, theta, tandist, xangle;


dist = sqrt(mathSquare(myState[0] - center[0]) +
                  mathSquare(myState[1] - center[1]) +
                  mathSquare(myState[2] - center[2]));


theta = asin(radius/dist);
tandist = dist * sin(theta);
xangle = atan2(center[1] - myState[1], center[0] - myState[0]);


tangentPoint[0] = myState[0] + (tandist * cos(theta + xangle));
tangentPoint[1] = myState[1] + (tandist * sin(theta + xangle));
tangentPoint[2] = 0;
//END::PROC::TangentFinder
}
static void doStrategy (char asteroidIsIndigens, char actionIsSpin, char stationIs1, float time, float myState[12])
{
//BEGIN::PROC::doStrategy
float indigens[3] = {0.0, 0.6, 0.0};
float opulens[3] = {0.0, -0.6, 0.0};
float station1[3] = {0.6, 0.0, 0.0};
float station2[3] = {-0.6, 0.0, 0.0};
float target[3] = {0.0,0.0,0.0};
float distvec[3] = {0.0, 0.0, 0.0};
float dist;
float PointsEarned;

PointsEarned = PgetScore() - Points;

DEBUG(("Time: %f, Points: %f, Fuel Used: %f\n", time, PointsEarned, (100-PgetPercentFuelRemaining())));


if(time >= 140)
{
    if(stationIs1)
    {
        ZRSetPositionTarget(station1);
    }
    else
    {
        ZRSetPositionTarget(station2);
    }
    return;
}

if(asteroidIsIndigens)
{
    if(actionIsSpin)
    {
        if(time >= 48)
        {
            spin(myState);
        }
        ZRSetPositionTarget(indigens);
    }
    else
    {
        mathVecSubtract(distvec, indigens, myState, 3);
        dist = mathVecMagnitude(distvec, 3);
        if(dist > 0.60)
        {
            TangentFinder(myState, indigens, 0.40, target);
            ZRSetPositionTarget(target);
        }
        else
        {
            OrbitDirectionally(indigens, 0.40, myState, 0);
        }
    }
   if(readyToLeaveOrbit(myState, indigens, station1) == 'y'){
        leaveOrbit(myState, indigens, station1);
    }
}
else//opulens
{
    if(actionIsSpin)
    {
        if(time >= 48)
        {
            spin(myState);
        }
        ZRSetPositionTarget(opulens);
    }
    else//problem is definitely in here
    {
        mathVecSubtract(distvec, opulens, myState, 3);
        dist = mathVecMagnitude(distvec, 3);
        if(dist > 0.60)
        {
            TangentFinder(myState, opulens, 0.40, target);
            ZRSetPositionTarget(target);
        }
        else
        {
            OrbitDirectionally(opulens, 0.40, myState, 0);
        }
    }
    if(readyToLeaveOrbit(myState, opulens, station1) == 'y')
    {
        leaveOrbit(myState, opulens, station1);
    }
}




    
//END::PROC::doStrategy
}
