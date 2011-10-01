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
static float Vfunc (int which, float *v1, float *v2, float *vresult, float scalar); //DECL::PROC::Vfunc
static float timeToMS (float myState[12], float station[3]); //DECL::PROC::timeToMS
static float shoot (float myState[12], float target[3], unsigned int fire, char shooter, unsigned int otherSatellite); //DECL::PROC::shoot

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
//BEGIN::PROC::ZRUser
#define VLen(a) mathVecMagnitude((a), 3) 
  //finds magnitude, returns float
#define VAdd(a, b, result) mathVecAdd(result, (a), (b), 3)  
  //adds vectors a and b and returns it in result
#define VSub(a, b, result) mathVecSubtract(result, (a), (b), 3) 
  //subtracts vectors a and b and returns it in res
#define VUnit(a) mathVecNormalize((a), 3))
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

float indigens[3] = {0.0, 0.6, 0.0};
float opulens[3] = {0.0, -0.6, 0.0};
float station1[3] = {0.6, 0.0, 0.0};
float station2[3] = {-0.6, 0.0, 0.0};
float laser1[3] = {0.4, 0.0, 0.0};
float laser2[3] = {-0.4, 0.0, 0.0};
float shield[3] = {0.0, 0.4, 0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3];
float *Station;
float *Asteroid;


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


Station = (SphereNumber == 1) ? station1 : station2; //station closer to where you started
Asteroid = asteroid ? opulens : indigens;


switch(state)
{
    case 0:
        ZRSetPositionTarget(((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? laser1 : laser2);
        shoot(myState, opulens, 0, 'L', 0);
        if(PhaveLaser() || PgetPhase() == 2)
            state = 1;
        break;
        
    case 1:
        if(!getShield)
            shield[1] = .3;
        ZRSetPositionTarget(shield);
        shoot(myState, opulens, 0, 'L', 0);
        if(PotherHasShield() || PhaveShield() || PgetPhase() == 2)
            state = 2;
        break;
        
    case 2:
        if(PiceHits() < 14 && !asteroid)
	shoot(myState, opulens, (PgetPhase() > 1), 'L', 0);
        if(!(PiceMelted()) && asteroid)
        {
            mathVecSubtract(difference, myState, opulens, 3);
            if(mathVecMagnitude(difference, 3) > .8)
                ZRSetPositionTarget(opulens);
            else
                ZRSetVelocityTarget(zero);
	      shoot(myState, opulens, (PgetPhase() > 1), 'L', 0);
            return;
        }
	orbit(myState, Asteroid, Station == station2);
	  if (PinAsteroid(myState) != PinAsteroid(otherState)) {
          DEBUG(("heretic!  burn him at the stake\n"));
          if (VDist(myState, otherState) < 1) {
              DEBUG(("he's close enough\n"));
          }
	      shoot(myState, otherState, VDist(myState, otherState) < 1, 'T', 1); // attract non cooperative opponent
	  }
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
static float shoot (float myState[12], float target[3], unsigned int fire, char shooter, unsigned int otherSatellite)
{
//BEGIN::PROC::shoot
float direction[3];
float att_target[3];
float myStateProjected[3];
float targetProjected[3];

// predict where I'll be in 2 seconds
VMult(&myState[3], 2, myStateProjected);
VAdd(myState, myStateProjected, myStateProjected);

// if target is the other satellite, find where he'll be in 2 seconds
if (otherSatellite) {
    VMult(&target[3], 2, targetProjected);
    VAdd(target, targetProjected, targetProjected);
}
else {
    VCopy(target, targetProjected);
}
VPoint(myStateProjected, targetProjected, att_target);
ZRSetAttitudeTarget(att_target);
mathVecSubtract(direction, target, myState, 3);

if (fire && (acos(mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) < (0.1))) {
    switch (shooter) {
        case 'L':
            Plaser();
            break;
        case 'T':
            Ptractor();
            break;
        case 'R':
            Prepulsor();
            break;
    }
}
else {
    DEBUG(("not in sight, or fire is false\n"));
}
//END::PROC::shoot
}
