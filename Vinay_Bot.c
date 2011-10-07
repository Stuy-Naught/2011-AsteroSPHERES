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
static float Station[3]; //DECL::VAR::Station
static float Asteroid[3]; //DECL::VAR::Asteroid
static char switchedStation; //DECL::VAR::switchedStation
static char op; //DECL::VAR::op
static void shoot (float myState[12], float target[3], unsigned char fire, unsigned char turn); //DECL::PROC::shoot

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
float opulens[3] = {0.0, -0.6, 0.0};
float Laser[3] = {0.4, 0.0, 0.0};
float zero[3] = {0.0, 0.0, 0.0};
float difference[3], theta;
float shield[3] = {0.0, 0.2, 0.0};
float ts = -0.66;

if(!time)
{
    SphereNumber = !!(myState[0] < 0) + 1;
    Station[0] = (SphereNumber == 1) ? 0.6 : -0.6;
}

Laser[0] = ((SphereNumber == 1) - (PotherHasLaser() == SphereNumber)) ? 0.4 : -0.4;

switch((int)PgetMessage())
{
    case 6:
        if(!(Station[0] < 0.0) && switchedStation++ < 3)
            Station[0] = -0.6;
        break;
    case 7:
        if(!(Station[0] > 0.0) && switchedStation++ < 3)
            Station[0] = 0.6;
        break;
    case 2:
    case 3:
        if(time < 40)
            op = 1;
        break;
    case 4:
    case 5:
        if(time < 40)
            op = 0;
        break;
}

if((time > 140) && (switchedStation == 0 || switchedStation > 2) && !(state==4)){
    mathVecSubtract(difference, Station, otherState, 3);
    Laser[0] = mathVecInner(&otherState[3], difference, 3) / (mathVecMagnitude(&otherState[3], 3) * mathVecMagnitude(difference, 3));
    Station[1] *= -1;
    mathVecSubtract(difference, Station, otherState, 3);
    Laser[1] = mathVecInner(&otherState[3], difference, 3) / (mathVecMagnitude(&otherState[3], 3)* mathVecMagnitude(difference, 3));
    if(Laser[1] < Laser[0])
        Station[1] *= -1;
}

if(time <= 120)
    PsendMessage(4 - 2*op + action);
else
    PsendMessage((Station[0] < 0) + 6);

Asteroid[1] = (op) ? -0.6 : 0.6;

//DEBUG(("cooloff = %d, timer = %d, action = %s, oppAttRate = %.1f\n", cooloff, switchTimer, (action) ? "orbit" : "spin", otherState[11]*180/PI));

switch(state)
{
    case 0:
        ZRSetPositionTarget(Laser);
        shoot(myState, opulens, 0, 1);
        state = !!PhaveLaser();
        if(PgetPhase()==2)
            state = 3;
        break;
        
    case 1:
        ZRSetPositionTarget(shield); // actually disruptor 
        shoot(myState, opulens, 0, 1);
        state = !!(PotherDisruptorUpgraded() || PdisruptorUpgraded()) + 1;
        if(PgetPhase()==2)
            state = 3;
        break;
        
    case 2:
        shield[1] = 0.4;
        ZRSetPositionTarget(shield);
        shoot(myState, opulens, 0, 1);
        state = !!(PotherHasShield() || PhaveShield() || PgetPhase() == 2) + 2;
        break;
        
    case 3:
        if(switchTimer == 4)
        {
            if(time > 120)
            {
                action = 0;
            }
            switchTimer = 10;
            cooloff = 5;
        }
        if(switchTimer == 14)
        {
            action = 1;
            switchTimer = 0;
            cooloff = 5;
        }
        
        if(cooloff)
            cooloff--;
        
        if(action)
        {
            if(!cooloff)
            {
                if(PisRevolving(otherState) == (-1*(op) + 2))
                {
                    if(switchTimer < 4 && switchTimer >= 0)
                        switchTimer++;
                    else
                        switchTimer = 1;
                }
                else
                    switchTimer = 0;
            }
            mathVecSubtract(difference, myState, Asteroid, 3);

            if(mathVecMagnitude(difference, 3) > .6)
                ZRSetPositionTarget(Asteroid);
            else
                ZRSetVelocityTarget(zero);

            if(!(PiceMelted()) && op){
                shoot(myState, opulens, (PgetPhase() == 2), 1);
                return;
            }
            theta = atan2f(difference[1],difference[0]);
            theta += ts;
            difference[0] = .4 * sinf(PI/2 - theta);
            difference[1] = .4 * sinf(theta);
            mathVecAdd(zero, Asteroid, difference, 3);//zero isn't 0,0,0 anymore
            ZRSetPositionTarget(zero);                
        }
        else
        {
            if(!cooloff)
            {
                mathVecSubtract(difference, otherState, Asteroid, 3);
                if(PinAsteroid(otherState) == (-1*(op) + 2) || ((mathVecMagnitude(difference, 3) < .2) && (fabsf(otherState[11]) > PI/12)))
                {
                    if(switchTimer < 14 && switchTimer >= 10)
                        switchTimer++;
                    else
                        switchTimer = 11;
                }
                else
                    switchTimer = 10;
            }
            
            if(!(PiceMelted()) && op)
            {
                shoot(myState, opulens, (PgetPhase() == 2), 1);
                return;
            }
            //spin
            zero[2] = 1.872459067 + atan2f(myState[7], myState[6]);
            zero[0] = sinf(PI/2 - zero[2]);
            zero[1] = sinf(zero[2]);
            zero[2] = 0;
            ZRSetAttitudeTarget(zero);
            ZRSetPositionTarget(Asteroid);
        }
        //In case you're on Indigens (!op) but didn't want to avoid spinning/orbiting in order to shoot
        if(PiceHits() < 14 && !PiceMelted())
            shoot(myState, opulens, (PgetPhase() == 2), action);
        mathVecSubtract(difference, Station, myState, 3);
        if(((((mathVecMagnitude(difference, 3) / 0.08) + 
            (acos(mathVecInner(&myState[3], difference, 3)/(mathVecMagnitude(&myState[3],3)*mathVecMagnitude(difference,3)))*10/PI) +
            ((0.08 - mathVecMagnitude(&myState[3], 3)) / .01) +
            4.0) > 168.0 - time) && action) || (time > 150 && !action))
            state++;
        break;
    
    default:
        mathVecSubtract(difference, Asteroid, myState, 3);
        mathVecSubtract(shield, Station, myState, 3);
        
        theta = mathVecMagnitude(shield, 3);
        
        if(theta < .2)
            ts = (mathVecMagnitude(&myState[3], 3) > 0.01) ? 0.002 : 0.01;
        else if(theta < 0.4)
            ts = theta/14;
        else{
            mathVecCross(zero, difference, &myState[3]);
            ts = mathVecMagnitude(difference, 3)*3.141592/(45*mathVecMagnitude(zero, 3));
        }
            
        ts = (ts > theta/9) ? theta/9 : ts;
        ts = (ts > 0.08) ? 0.08 : ts;
        
        mathVecNormalize(shield, 3);
        
        shield[0] *= ts;
        shield[1] *= ts;
        
        ZRSetVelocityTarget(shield);
        break;
}
if((PgetCharge() > 0) && action && (time > 60) && (!op || PiceMelted()) && (PiceHits() >= 14)){
    mathVecSubtract(difference, otherState, myState, 3);
    ZRSetAttitudeTarget(difference);
    if(((mathVecInner(&myState[6], difference, 3) / mathVecMagnitude(&myState[6], 3)) > (0.994522))&&(mathVecMagnitude(difference, 3)<0.5))
        Prepulsor();
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
memset(Station,0,sizeof(float)*3);
memset(Asteroid,0,sizeof(float)*3);
switchedStation = 0;
op = 1;
//END::PROC::ZRInit
}
//User-defined procedures
static void shoot (float myState[12], float target[3], unsigned char fire, unsigned char turn)
{
//BEGIN::PROC::shoot
float direction[3];

mathVecSubtract(direction, target, myState, 3);
mathVecNormalize(direction, 3);

if(turn)
    ZRSetAttitudeTarget(direction);

if (fire && ((mathVecInner(&myState[6], direction, 3) / mathVecMagnitude(&myState[6], 3)) > (0.994522)))
    Plaser();
//END::PROC::shoot
}
