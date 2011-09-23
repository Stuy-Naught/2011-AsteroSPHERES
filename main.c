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

static float Vfunc (int which, float *v1, float *v2, float *vresult, float scalar); //DECL::PROC::Vfunc

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

float xaxis[3] = {1.0, 0.0, 0.0};
float vector1[3] = {0.3, 0.4, -0.2};
float vector2[3] = {0.0, 0.3, 0.4};
float vector3[3] = {0.0, 0.3, 0.4};
float resultant[3] = {0.0, 0.0, 0.0};
float result = 0.0;
float scale = 2.0;

result = VLen(vector1);
DEBUG(("%s, %f\n", "VLen", result));

VAdd(vector1, vector2, resultant);
prvect("VAdd", resultant);

VSub(vector1, vector2, resultant);
prvect("VSub", resultant);

VUnit(vector2);
prvect("VUnit", vector2);

VMult(vector3, scale, resultant);
prvect("VMult", resultant);

result = VDot(vector1, vector3);
DEBUG(("%s, %f\n", "VDot", result));

VCross(vector1, vector3, resultant);
prvect("VCross", resultant);

result = VDist(vector1, vector3);
DEBUG(("%s, %f\n", "VDist", result));

VCopy(vector3, resultant);
prvect("VCopy", resultant);

result = VAngle(xaxis, vector1);
DEBUG(("%s, %f\n", "VAngle", result));

VPoint(xaxis, vector3, resultant);
prvect("VPoint", resultant);



//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit

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
