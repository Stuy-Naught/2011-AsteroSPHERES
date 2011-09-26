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

static unsigned int headed; //DECL::VAR::headed
static int opponentHeaded (float otherState[12]); //DECL::PROC::opponentHeaded
static float pos (int obj, int dim); //DECL::PROC::pos

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
int oppHeaded;
float dest[2];
oppHeaded = opponentHeaded(otherState);
dest[0] = pos(oppHeaded, 0);
dest[1] = pos(oppHeaded, 1);
if(dest[0] == 1.0){
dest[0] = myState[0];
dest[1] = myState[1];
}
ZRSetPositionTarget(dest);
DEBUG(("The opponent is headed toward %d\n", oppHeaded));
DEBUG(("Past destinations are: %d\n", headed));
//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
headed = 0;
//END::PROC::ZRInit
}
//User-defined procedures
static int opponentHeaded (float otherState[12])
{
//BEGIN::PROC::opponentHeaded
/*Returns 8 if unsure of opponent's direction,
/ 0 if Laser1,
/ 1 if Laser2,
/ 2 if Disruptor Upgrade
/ 3 if Shield
/ 4 if Indigens
/ 5 if Opulens
/ 6 if Mining Base 1
/ 7 if Mining Base 2
*/
int defHeaded;

int min(float *angles, int length){
  int i;
  int ans = 0;
  for(i = 1; i < length; i++){
    if(angles[i] < angles[ans])
      ans = i;
  }
  return ans;

}




float angleTo(int object){

  float oppVel[2];
  float dir[2];

  oppVel[0] = otherState[3];
  oppVel[1] = otherState[4];

  dir[0] = pos(object,0) - otherState[0];
  dir[1] = pos(object,1) - otherState[1];

  mathVecNormalize(oppVel, 2);
  mathVecNormalize(dir, 2);

  return acos(oppVel[0] * dir[0] + oppVel[1] * dir[1]);

}

int headedTo(){
  float anglesToObjects[8];
  int j;
  for(j = 0; j < 8; j++)
    anglesToObjects[j] = angleTo(j);

  return min(anglesToObjects, 8);

}


int dig(int num, int digit){
  if(digit == 0)
    return num % 10;
  else
    return dig(num / 10, digit - 1);
}

headed /= 10;
headed += (headedTo() * 10000000);
if((dig(headed, 7)  == dig(headed, 6)) && (dig(headed, 6) == dig(headed, 5)))
  defHeaded = headedTo();
else
  defHeaded = 8;
  
return defHeaded;
//END::PROC::opponentHeaded
}
static float pos (int obj, int dim)
{
//BEGIN::PROC::pos

  float ansPos[2];
  switch(obj){
    case 0 : ansPos[0] = -0.4;
             ansPos[1] = 0.0;
             break;
    case 1 : ansPos[0] = 0.4;
             ansPos[1] = 0.0;
             break;
    case 2 : ansPos[0] = 0.0;
             ansPos[1] = 0.2;
             break;
    case 3 : ansPos[0] = 0.0;
             ansPos[1] = 0.4;
             break;
    case 4 : ansPos[0] = 0.0;
             ansPos[1] = 0.6;
             break;
    case 5 : ansPos[0] = 0.0;
             ansPos[1] = -0.6;
             break;
    case 6 : ansPos[0] = 0.6;
             ansPos[1] = 0.0;
             break;
    case 7 : ansPos[0] = -0.6;
             ansPos[1] = 0.0;
             break;
    default : ansPos[0] = 1.0;
              ansPos[1] = 1.0;
  }

  return ansPos[dim];


//END::PROC::pos
}
