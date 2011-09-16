#include <math.h>
#include <string.h>
#include "ZR_API.h"
#include "ZRGame.h"
#include "math_matrix.h"
#include "spheres_constants.h"
static float procvar[12];
static void rotatePoint2(float* point1, float theta, float* newPoint);
 
//User01: guest3 Team: guest3 Project: centripetal circle
void ZRUser01(float *myState, float *otherState, float time)
{
#define PI  3.141593
#define TAU (2*PI)
 
#define posTarget         (&procvar[0])
#define center            (&procvar[3])
#define current_radius    (&procvar[6])
#define radius            (procvar[9])
#define angleStep         (procvar[10])
#define currentAngle      (procvar[11])
 
int i;
 
if (time < 1) {
    memset(procvar, 0, 13);
    radius = 0.5;
    angleStep = TAU / 32;
}
 
posTarget[0] = radius * cos(currentAngle);
posTarget[1] = radius * sin(currentAngle);
 
if (sqrt(mathSquare(myState[0] - posTarget[0]) +
         mathSquare(myState[1] - posTarget[1]) +
         mathSquare(myState[2] - posTarget[2])) < 0.05) {
    currentAngle += angleStep;
}
 
ZRSetPositionTarget(posTarget);
 
// connect radius from center to current position
for (i = 0; i < 3; i++) {
    current_radius[i] = myState[i] - center[i];
}
 
DEBUG(("time: %2f\tcurrent_radius: %f,\tcurrentAngle: %f\n",
       time,
       mathVecMagnitude(current_radius, 3),
       currentAngle));
}
void ZRInit01()
{
  memset(procvar,0,sizeof(float)*12);
 
}
//User-defined procedures
 
static void rotatePoint2(float* point1, float theta, float* newPoint)
{
#define X point1[0]
#define Y point1[1]
#define NEW_X newPoint[0]
#define NEW_Y newPoint[1]
 
NEW_X = X * cos(theta) - Y * sin(theta);
NEW_Y = X * sin(theta) + Y * cos(theta);
}
