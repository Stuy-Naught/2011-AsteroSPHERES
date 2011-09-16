#include <math.h>
#include <string.h>
#include "ZR_API.h"
#include "ZRGame.h"
#include "math_matrix.h"
#include "spheres_constants.h"
static float procvar[12];
static float pointTotal;
static void rotatePoint2(float* point1, float theta, float* newPoint);

//User01: guest3 Team: guest3 Project: centripetal circle
void ZRUser01(float *myState, float *otherState, float time)
{
#define PI 3.141593
#define TAU (2*PI)

#define posTarget         (&procvar[0])
#define center            (&procvar[3])
#define current_radius    (&procvar[6])
#define radius            (procvar[9])
#define angleStep         (procvar[10])
#define angleTarget       (procvar[11])
 
int i;
float speed;
float angularVelocity;
float desiredAngularVelocity;
float points;
float velTarget[3];
 
if (time < 1) {
    memset(procvar, 0, 13);
    radius = 0.5;
    angleStep = TAU / 16;
    DEBUG(("time, current_radius, angularVelocity, angleTarget\n"));
}
 
desiredAngularVelocity = 4.0 * PI / 180;
 
posTarget[0] = radius * cos(angleTarget);
posTarget[1] = radius * sin(angleTarget);
 
speed = mathVecMagnitude(&myState[3], 3); // magnitude of the velocity vector
angularVelocity = speed / radius;
points = (0.05 / 4) * (4 - fabs(4 - (angularVelocity * 180 / PI)));
pointTotal = pointTotal + points;
 
if (sqrt(mathSquare(myState[0] - posTarget[0]) +
         mathSquare(myState[1] - posTarget[1]) +
         mathSquare(myState[2] - posTarget[2])) < 0.05) {
    angleTarget += angleStep;
}
 
velTarget[0] = posTarget[0] - myState[0];
velTarget[1] = posTarget[1] - myState[1];
velTarget[2] = posTarget[2] - myState[2];
mathVecNormalize(velTarget, 3);
velTarget[0] *= radius * desiredAngularVelocity;
velTarget[1] *= radius * desiredAngularVelocity;
velTarget[2] *= radius * desiredAngularVelocity;
 
ZRSetVelocityTarget(velTarget);
 
// connect radius from center to current position
for (i = 0; i < 3; i++) {
     current_radius[i] = myState[i] - center[i];
}
 
DEBUG(("%f, %f, %f, %f, %f\n",
       time,
       mathVecMagnitude(current_radius, 3),
       angularVelocity,
       angleTarget,
       pointTotal));
}
void ZRInit01()
{
  memset(procvar,0,sizeof(float)*12);
  pointTotal = (float) 0.0;
 
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
