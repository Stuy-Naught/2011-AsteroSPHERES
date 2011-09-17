#include <math.h>
#include <string.h>
#include "ZR_API.h"
#include "ZRGame.h"
#include "math_matrix.h"
#include "spheres_constants.h"
 
//User01: guest1 Team: Stuy-rules! Project: 2D_Positioning
void ZRUser01(float *myState, float *otherState, float time)
{
//
//SphereTarget[3] can be changed to make a different destination.
//
 
/*
 
SPHERE1 spawn at
[0.4, -0.6]
[0.4, -0.6, 0.0]
 
Disruptor Upgrade
[0.0,0.2]
[0.0,0.0,-0.4]
 
Shield
[0.0,0.4]
[0.0,0.0,0.4]
 
Laser 1
[0.4,0.0]
[0.4,0.0,0.0]
 
Laser 2
[-0.4,0.0]
[-0.4,0.0,0.0]
 
Indigens
[0.0,0.6]
[0.0, 0.35, 0.2]
 
Opulens
[0.0,-0.6]
[0.0,-0.35, -0.2]
 
Mining Station 1
[0.6,0.0]
[0.6,0.0,-0.5]
 
Mining Station 2
[-0.6,0.0]
[0.6,0.0,0.5]
 
*/
 
float laser1[3] = {0.4, 0.0, 0.0};
 
float SphereTarget[3] = {0.0, 0.0, 0.0};
 
SphereTarget[0]= laser1[0];
SphereTarget[1]= laser1[1];
SphereTarget[2]= laser1[2];
 
ZRSetPositionTarget(SphereTarget);
 
DEBUG(("time is %f\n fuel is %f\n distance to target is%f\n",
time, getPercentFuelRemaining(),
sqrt(mathSquare(myState[0]-SphereTarget[0]) + mathSquare(myState[1]-SphereTarget[1]) + mathSquare(myState[2]-SphereTarget[2]))));
 
 
}
void ZRInit01()
{
 
}
//User-defined procedures
