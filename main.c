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
static char action; //DECL::VAR::action
static char switchTimer; //DECL::VAR::switchTimer
static char cooloff; //DECL::VAR::cooloff
static int gotToTangent; //DECL::VAR::gotToTangent
static float TAU; //DECL::VAR::TAU
static float prevPoint[3]; //DECL::VAR::prevPoint
static void shoot (float myState[12], float target[3], unsigned int fire); //DECL::PROC::shoot
static void Rotate3D (float *center, float *axis, float *point, float angle, float *new_point, float radius); //DECL::PROC::Rotate3D
static void RevolveWaypoints (float *myState, float *waypoint); //DECL::PROC::RevolveWaypoints

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
#define thetastep   dist
#define theta       mag
#define dirs        Laser

  float opulens[3] = {0.0, -0.35, -0.2}, opulens_spin_point[3] = {0.04, -0.35, -0.2}, Laser[3] = {0.4, 0.0, 0.0}, difference[3], target[3], radius = .35, posTarget[3], angleStep, asteroidNormal[3];
    
  
  if(!time)
      SphereNumber = !!(myState[0] < 0) + 1;

  Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;
  
   
      if(((int)PgetMessage() & 0x0400) && (time < 40)) // y0b0tics: both of us are fully capable, so SPH1 orbits, SPH2 spins
	{
	  action = SphereNumber % 2;
	}
      else if(((int)PgetMessage() & 0x0200) && (time < 40)) // y0b0tics: they're not as capable, so we orbit
	// not specific enough, delta mk2 should tell us exactly what they can/cannot do
	{
	  action = 1;
	}
      
    
  if(time != 1 && time != 2)
      PsendMessage(0x0400); // y0b0tics: "I can do everything"
    
 
  
         PgetAsteroidNormal(asteroidNormal);
  
if(!state){
      ZRSetPositionTarget(Laser);
      shoot(myState, opulens, 0);
      if(PhaveLaser() || PgetPhase() == 2)
	{
	  state = 1;
	}
}
else{
    if(switchTimer == 5)
	{
	  if(time < 130)
	    {
	      action = 0;
	    }
	  switchTimer = 10;
	  cooloff = 8;
	}
      if(switchTimer == 15)
	{
      if(time < 130)
        {
	      action = 1;
        }
	  switchTimer = 0;
	  cooloff = 8;
	}
			      
      if(cooloff)
	    {
	    cooloff--;
	    }
				
      if(action)
	    {
	  if(!cooloff)
	    {
          mathVecSubtract(difference, opulens, otherState, 3);
          mathVecNormalize(difference, 3);
	      if((PisRevolving(otherState) == 1) && (mathVecInner(&otherState[3], difference, 3) < (0.9 * mathVecMagnitude(&otherState[3], 3))))
		{
		  if(switchTimer < 5 && switchTimer >= 0)
		    {
		      switchTimer++;
		    }
		  else
		    {
		      switchTimer = 1;
		    }
		}
	      else
		{
		  switchTimer = 0;
		}
	    }
				      
	  mathVecSubtract(difference, myState, opulens, 3);
					
	  if((!PiceMelted()))
	    {
	      shoot(myState, opulens, (PgetPhase() > 1));
	    }
					  


	  if (fabsf(mathVecMagnitude(difference, 3) - 0.35) < (0.03 + gotToTangent / 5))
        {
        gotToTangent = 1;
               
         //tweak opulens as neccesary
       angleStep = TAU / 88.5;
       
       Rotate3D(opulens,asteroidNormal,myState,angleStep,posTarget,radius);
       
       
       ZRSetVelocityTarget(posTarget);
        
        
	    }
      else
        {
            RevolveWaypoints(myState, target);
        ZRSetPositionTarget(target);
        }
	}
      else
	{
	  if(!cooloff)
	    {
	      mathVecSubtract(difference, otherState, opulens, 3);
	      if(!(PisRevolving(otherState) == 1))
		{
		  if(switchTimer < 15 && switchTimer >= 10)
		    {
		      switchTimer++;
		    }
		  else
		    {
		      switchTimer = 11;
		    }
		}
	      else
		{
		  switchTimer = 10;
		}
	    }

	  if(!(PiceMelted()))
	    {
	      opulens_spin_point[0] = (SphereNumber == 1) ? 0.04 : -0.04;
	      ZRSetPositionTarget(opulens_spin_point);
	      shoot(myState, opulens, (PgetPhase() > 1));
          RevolveWaypoints(myState, prevPoint);
	      return;
	    }
        angleStep = TAU / 12;
       Rotate3D(opulens,asteroidNormal,&myState[6],angleStep,posTarget,radius);
        //mathVecNormalize(posTarget, 3);
	   ZRSetAttitudeTarget(posTarget);
						  
	    ZRSetPositionTarget(opulens);

      
      
	}
}
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
state = 0;
SphereNumber = 0;
action = 1;
switchTimer = 0;
cooloff = 0;
gotToTangent = 0;
TAU = 6.28318;
memset(prevPoint,0,sizeof(float)*3);
//END::PROC::ZRInit
}
//User-defined procedures
static void shoot (float myState[12], float target[3], unsigned int fire)
{
//BEGIN::PROC::shoot
float direction[3];
float new_pos[3];

// new_pos:  predict my new position in 2 seconds
mathVecAdd(new_pos, &myState[3], &myState[3], 3); 
mathVecAdd(new_pos, myState, new_pos, 3);

// direction: where should I aim in the future
mathVecSubtract(direction, target, new_pos, 3);
mathVecNormalize(direction, 3);
      
ZRSetAttitudeTarget(direction);

// direction: where should I be pointing NOW
mathVecSubtract(direction, target, myState, 3);
mathVecNormalize(direction, 3);

// shoot if I "see" the target
if (fire && (mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) > (0.995))
  Plaser();

//END::PROC::shoot
}
static void Rotate3D (float *center, float *axis, float *point, float angle, float *new_point, float radius)
{
//BEGIN::PROC::Rotate3D
/* Implements the general 3D Rotation Matrix approach.
Usage:
center: position of the asteroid.
axis: is the normalized (unit vector) of the rotation axis.
point: is the position of the satellite (you can use myState).
angle: is rotation angle in radians.
new_point: will be filled with the new position.

-- Peter Brooks, 10/22/11
*/

float sina, cosa, cos1a;
float e0, e1, e2, e00, e01, e02, e11, e12, e22;
float point2[3];
float row[3][3];
int i;


e0=axis[0];
e1=axis[1];
e2=axis[2];
e00=e0*e0;
e01=e0*e1;
e02=e0*e2;
e11=e1*e1;
e12=e1*e2;
e22=e2*e2;

sina = sinf(angle);
cosa = cosf(angle);
cos1a = 1.0 - cosa;

// move center of rotation to origin, and so find translated value of point
mathVecSubtract(point2, point, center, 3);

// Calculate the rows of the Rotation matrix
row[0][0] = cosa + e00*cos1a;
row[0][1] = e01*cos1a - e2*sina;
row[0][2] = e02*cos1a + e1*sina;

row[1][0] = e01*cos1a + e2*sina;
row[1][1] = cosa + e11*cos1a;
row[1][2] = e12*cos1a - e0*sina;

row[2][0] = e02*cos1a - e1*sina;
row[2][1] = e12*cos1a + e0*sina;
row[2][2] = cosa + e22*cos1a;

// multiply the Rotation Matrix by point2 to produce the rotated position in point3
for (i = 0; i < 3; ++i) {
    new_point[i] = mathVecInner(row[i],point2,3);//+center[i];
}

mathVecNormalize(new_point,3);

new_point[0] = new_point[0] * radius + center[0] - point[0];
new_point[1] = new_point[1] * radius + center[1] - point[1];
new_point[2] = new_point[2] * radius + center[2] - point[2];
mathVecNormalize(new_point, 3);
new_point[0] *= ((TAU / 75) * radius);
new_point[1] *= ((TAU / 75) * radius);
new_point[2] *= ((TAU / 75) * radius);

//END::PROC::Rotate3D
}
static void RevolveWaypoints (float *myState, float *waypoint)
{
//BEGIN::PROC::RevolveWaypoints
float opulens[3]={0, -.35,-.2};
float relativePosition[3];
float equatorPoint[3];
float radius=.35;
float perpendicular[3];
float axis[3];
//float waypoint[3];

//sets the asteroid axis to the float array "axis"
PgetAsteroidNormal(axis);

//finds your sphere's position relative to opulens and puts it into the float array "relativePosition"
mathVecSubtract(relativePosition,myState,opulens,3);

//finds the equator of opulens by doing 2 cross products and puts it into the float array "equatorPoint"
mathVecCross(perpendicular,axis,relativePosition);
mathVecCross(equatorPoint,perpendicular,axis);

//sets "equatorPoint" to the length of the orbiting radius
mathVecNormalize(equatorPoint,3);
equatorPoint[0]*=radius;
equatorPoint[1]*=radius;
equatorPoint[2]*=radius;

/*finds the waypoint, the float array of "waypoint" 
by adding the coordinates of opulens and the equator point*/
mathVecAdd(waypoint,equatorPoint,opulens,3);
//END::PROC::RevolveWaypoints
}
