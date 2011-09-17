#include <math.h>
#include <string.h>
#include "ZR_API.h"
#include "ZRGame.h"
#include "math_matrix.h"
#include "spheres_constants.h"
 
//User01: guest1 Team: Stuy-rules! Project: 2D_Positioning
void ZRUser01(float *myState, float *otherState, float time)
{
//sphere1 to laser1
//spawn at (0.4, -0.6)
//laser1 at (0.4, 0.0)
 
float laser1P[3] = {0.4, 0.0, 0.0};
ZRSetPositionTarget(laser1P);
 
DEBUG(("time is %f\n fuel is %f\n distance to laser is%f\n",
time, getPercentFuelRemaining(),
sqrt(mathSquare(myState[0]-0.4) + mathSquare(myState[1]))));
 
 
}
void ZRInit01()
{
 
}
//User-defined procedures
