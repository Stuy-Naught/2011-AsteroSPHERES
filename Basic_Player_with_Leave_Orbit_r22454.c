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
static float Score; //DECL::VAR::Score
static int state; //DECL::VAR::state
static float Vfunc (int which, float *v1, float *v2, float *vresult, float scalar); //DECL::PROC::Vfunc
static char readyToLeaveOrbit (float myState[12], float asteroid[3], float mBase[3]); //DECL::PROC::readyToLeaveOrbit
static void Orbit (float center[3], float radius, float myState[12]); //DECL::PROC::Orbit
static float pos (int obj, int dim); //DECL::PROC::pos
static void leaveOrbit (float myState[12], float asteroid[3], float mPanel[3]); //DECL::PROC::leaveOrbit

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
#define VLen(a) mathVecMagnitude((a), 3) 
  //finds magnitude, returns float
#define VAdd(a, b, result) mathVecAdd(result, (a), (b), 3)  
  //adds vectors a and b and returns it in result
#define VSub(a, b, result) mathVecSubtract(result, (a), (b), 3) 
  //subtracts vectors a and b and returns it in res
#define VUnit(a) mathVecNormalize((a), 3) 
  //turns the vector into a unit vector
#define VMult(a, b, result) Vfunc(4, (a), NULL, (result), (b))
  //multiplies vector a by scalar b and returns it in result
#define VDot(a, b) mathVecInner((a), (b), 3) 
  //finds dot product of vectors a and b
#define VCross(a, b, result) mathVecCross(result, (a), (b)) 
  //finds a x b and returns it in result
#define VDist(a, b) Vfunc(6, (a), (b), NULL, 0) 
  //returns the distance between points a and b (which is just the magnitude of a - b)
#define VCopy(a, result) memcpy((result), (a), sizeof(float)*3) 
  //copies array a into array b
#define VAngle(a, b) Vfunc(8, (a), (b), NULL, 0) 
  //returns the angle between two vecotrs
#define VPoint(a, b, result) Vfunc(9, (a), (b), result, 0) 
  //finds the unit vector pointing from point a to point b and returns it in result 
#define VRotate(a, b, result, deg) Vfunc(10, (a), (b), result, deg) 
  //rotates vector a towards vector b by (deg) degrees
  //deg has to be smaller than the angle between a and b (!)
  //returns 0 if sucessful, 1 if deg is too large
#define prvect(msg,p) DEBUG(("%s (%f,%f,%f)\n", msg, p[0], p[1], p[2]))
 
  float center[3];
  float radius;
  float circleStartPoint[3];
  float distanceToCircle[3];
  float Dstation1; //distance from current position to station 1
  float Dstation2;
  float station1[3];//coords of station1
  float station2[3];//coords of station2
  float speed;
  float initDest[3];
  
  initDest[0] = pos(0,0);
  initDest[1] = pos(0,1);
  initDest[2] = 0.0;
  
  station1[0] = 0.6;
  station1[1] = 0.0;
  station1[2] = 0.0;
  station2[0] = -0.6;
  station2[1] = 0.0;
  station2[2] = 0.0;
  
  center[0] = 0;
  center[1] = 0.6;
  center[2] = 0;
  
  radius = .35;
  
  circleStartPoint[0] = 0.4;
  circleStartPoint[1] = 0.6;
  circleStartPoint[2] = 0;
  
  
  if (time < 1) {
    DEBUG(("time, speed\n"));
  }
  if(-1 == state){
      ZRSetPositionTarget(initDest);
      if(VDist(initDest, myState) < 0.05 && VLen(&myState[3]) < 0.01)
        state = 0;
  }
      
  if (0 == state) {
    float velTarget[3];
    VPoint(myState, circleStartPoint, velTarget);
    VMult(velTarget, radius * TAU / 45, velTarget);
    ZRSetVelocityTarget(velTarget);
    if (VDist(myState, circleStartPoint) < 0.04) {
     state = 1;
     DEBUG(("Got to the circle!"));
    }
  }
  if (1 == state) {
    Orbit(center, radius, myState);
    if(time >= 140) {
      if(readyToLeaveOrbit(myState, center, station1) == 'y')
        state = 2;
      else if(readyToLeaveOrbit(myState, center, station2) == 'y')
        state = 3;
    }
    if(time >= 154) {    // Just go to closer station
        state = (VDist(myState, station1) < VDist(myState, station2) ? (2) : (3));
    }
  }
  if (2 == state) {
    leaveOrbit(myState, center, station1);
    DEBUG(("Going to Mining Base 1\n"));
  }
  if (3 == state) {
    leaveOrbit(myState, center, station2);
    DEBUG(("Going to Mining Base 2\n"));
  }
  speed = mathVecMagnitude(&myState[3], 3); // magnitude of the velocity vector
  
  DEBUG(("%f, %f\n",
     time,
   speed));
   
   
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
TAU = 6.28318;
Score = 0;
state = -1;
//END::PROC::ZRInit
}
//User-defined procedures
static float Vfunc (int which, float *v1, float *v2, float *vresult, float scalar)
{
//BEGIN::PROC::Vfunc
int i;

if (which == 4) { // vresult = scalar * v1
  for (i = 0; i < 3; ++i)
    vresult[i] = scalar * v1[i];
  return 0;
}

if (which == 6) { // returns distance between v1 and v2
  float v3[3];
  mathVecSubtract(v3, v1, v2, 3);  // v3 = v1 - v2
  return mathVecMagnitude(v3,3);
}

if (which == 8) { // angle between two vectors
  float dot = mathVecInner(v1, v2, 3)/(mathVecMagnitude(v1,3)*mathVecMagnitude(v2,3));
  return acos(dot)*180.0/3.14159265;
}

if (which == 9) { // unit vector pointing from v1 toward v2
  float v9[3];
  mathVecSubtract(v9, v2, v1, 3);
  memcpy(vresult, v9, sizeof(float)*3);
  mathVecNormalize(vresult,3);
  return 0;
}

if (which == 10) { // rotated vector v1 towards v2 at scalar degrees
  float ang = scalar*3.14159265/180;
  float u[3];
  float c;
  float s;
  float rot[3];
  int k;
  mathVecNormalize(v1,3);
  mathVecNormalize(v2,3);

  if (scalar > Vfunc(8,v1,v2,NULL,0)) {
    return 1;
  }

  c = cosf(ang);
  s = sinf(ang);
  mathVecCross(u,v1,v2);
  mathVecNormalize(u,3);

  for(k=0;k<2;k++) {
    if ((k == 1) && (fabs(Vfunc(8,rot,v1,NULL,0)) > fabs(Vfunc(8,v1,v2,NULL,0)) || fabs(Vfunc(8,rot,v2,NULL,0)) > fabs(Vfunc(8,v1,v2,NULL,0)))) {
  c = cosf(-1*ang);
s = sinf(-1*ang);
    }
    rot[0] = (v1[0]*(c + u[0] * u[0] * (1-c)) + v1[1]*(u[0] * u[1] * (1-c) - u[2] * s) + v1[2]*(u[0] * u[2] * (1-c) + u[1] * s));
    rot[1] = (v1[0]*(u[0] * u[1] * (1-c) + u[2] * s) + v1[1]*(c + u[1] * u[1] * (1-c)) + v1[2]*(u[1] * u[2] * (1-c) - u[0] * s));
    rot[2] = (v1[0]*(u[0] * u[2] * (1-c) - u[1] * s) + v1[1]*(u[1] * u[2] * (1-c) + u[0] * s) + v1[2]*(c + u[2] * u[2] * (1-c))); 
  }

  memcpy(vresult,rot,sizeof(float)*3);
  return 0;
}
//END::PROC::Vfunc
}
static char readyToLeaveOrbit (float myState[12], float asteroid[3], float mBase[3])
{
//BEGIN::PROC::readyToLeaveOrbit
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
static void Orbit (float center[3], float radius, float myState[12])
{
//BEGIN::PROC::Orbit

float desiredAngularVelocity;
float velTarget[3];
float angleTarget;
float posTarget[3];
float currentAngle;
float angleStep;
float pointsEarned;
float currentAngularVelocity;

//angular velocity = vsinx / r
    
    
pointsEarned = PgetScore() - Score;

currentAngle = atan2(myState[1] - center[1], myState[0] - center[0]);

currentAngularVelocity = (sqrt(pow(myState[3], 2) + pow(myState[4], 2))) * sin(currentAngle) / radius;

//DEBUG(("Time:%f, Points gotten: %f, Current Angular Velocity: %f\n", time, pointsEarned, currentAngularVelocity));


  //tweak angleStep as neccesary
angleStep = TAU * radius / 2;

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

DEBUG(("Points gotten: %f, Current AV: %f", pointsEarned, currentAngularVelocity));


Score = PgetScore();
ZRSetVelocityTarget(velTarget);
//END::PROC::Orbit
}
static float pos (int obj, int dim)
{
//BEGIN::PROC::pos

float ansPos[2];
switch(obj){
  case 0 : ansPos[0] = 0.4; //laser1
           ansPos[1] = 0.0;
           break;
  case 1 : ansPos[0] = -0.4; //laser2
           ansPos[1] = 0.0;
           break;
  case 2 : ansPos[0] = 0.0; //disruptor
           ansPos[1] = 0.2;
           break;
  case 3 : ansPos[0] = 0.0; //shield
           ansPos[1] = 0.4;
           break;
  case 4 : ansPos[0] = 0.0; //indigens
           ansPos[1] = 0.6;
           break;
  case 5 : ansPos[0] = 0.0; //opulens
           ansPos[1] = -0.6;
           break;
  case 6 : ansPos[0] = 0.6; //mb1
           ansPos[1] = 0.0;
           break;
  case 7 : ansPos[0] = -0.6; //mb2
           ansPos[1] = 0.0;
           break;
  default : ansPos[0] = 1.0; //what!?
            ansPos[1] = 1.0;
}

return ansPos[dim];


//END::PROC::pos
}
static void leaveOrbit (float myState[12], float asteroid[3], float mPanel[3])
{
//BEGIN::PROC::leaveOrbit
#define Angle(v1, v2) acosf(mathVecInner((v1), (v2), 3)/(mathVecMagnitude((v1), 3) * mathVecMagnitude((v2), 3)))
float targetVel[3];
float r, t;
float desiredMag;
float vecToAst[3];
int i;
float earth[] = {0,1,0};

for(i = 0; i < 3; i++)
    vecToAst[i] = asteroid[i] - myState[i];

for(i = 0; i < 3; i++)
    targetVel[i] = mPanel[i] - myState[i];
    
mathVecNormalize(targetVel, 3);

r = sqrt(mathVecInner(vecToAst, vecToAst, 3));

if(VDist(myState, mPanel) < 0.05){
    desiredMag = 0;
    Plaser();
    DEBUG(("FIRED AT EARTH!"));
}
else if(r > 0.6 || VDist(myState, mPanel) < 0.3){
    desiredMag = (VDist(myState, mPanel) - 0.1) / 20;
}
else{
    desiredMag = r*3.141592/(45*sinf(Angle(vecToAst, targetVel)));
}


if(desiredMag >= 0.065)
    desiredMag = 0.065;
    
for(i = 0; i < 3; i++)
    targetVel[i] *= sqrt(desiredMag);   

ZRSetVelocityTarget(targetVel);
ZRSetAttitudeTarget(earth);

//END::PROC::leaveOrbit
}
