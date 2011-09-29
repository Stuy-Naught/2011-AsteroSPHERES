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

static unsigned char SN_2D_data[60]; //DECL::VAR::SN_2D_data
static int SN_Failed; //DECL::VAR::SN_Failed
static int SN_2D_Setup (char *preferences, int sphere_num, float time); //DECL::PROC::SN_2D_Setup
static int SN_2D_Proceed (float *myState, float *otherState, float time); //DECL::PROC::SN_2D_Proceed
static int SN_2D_Result (int which); //DECL::PROC::SN_2D_Result

void ZRUser01(float *myState, float *otherState, float time)
{
//BEGIN::PROC::ZRUser
// Compile and run test
char preferences[] = {0,1,2,3,4,5,6,7,8,9,10,11};

// Just call each routine to force the simulation to run the code...  that's all.
SN_2D_Setup(preferences,1,time);
SN_2D_Proceed(myState,otherState,time);
SN_2D_Result(1);


//END::PROC::ZRUser
}
void ZRInit01()
{
//BEGIN::PROC::ZRInit
memset(SN_2D_data,0,sizeof(unsigned char)*60);
SN_Failed = 0;
//END::PROC::ZRInit
}
//User-defined procedures
static int SN_2D_Setup (char *preferences, int sphere_num, float time)
{
//BEGIN::PROC::SN_2D_Setup
// ====================== # defines ================================
// The #defines used by all SN_2D routines
#define SN_data_size    60      // this must be at least: 12 + 3*SN_num_prefs
#define SN_num_prefs    12
#define SN_want_to_use_2D_protocol     1729

// steps through the algorithm
#define SN_step_check_incoming_agreement     1
#define SN_step_start_sending_prefs		    1
#define SN_step_start_receiving_prefs		2
        // recalculate below if SN_num_prefs changes
#define SN_step_end_sending_prefs		    5  
#define SN_step_end_receiving_prefs         6
#define SN_step_conflict_resolution		    6
#define SN_step_final_processing		    7

// positions inside the SN_2D_data array
#define SN_step_num     0
#define SN_start_time   1
#define SN_sphere_num   2
#define SN_my_asteroid  3
#define SN_my_mining    4
#define SN_my_station   5
#define SN_other_asteroid   6
#define SN_other_mining     7
#define SN_other_station    8
#define SN_num_ties         9
#define SN_randnum_for_station    10
#define SN_randnum_for_tie        11
#define SN_my_prefs     12
        // recalculate below if SN_num_prefs changes
#define SN_other_prefs  24
#define SN_ties         36

#define SN_result_my_asteroid   0
#define SN_result_my_mining     1
#define SN_result_my_station    2
#define SN_result_other_asteroid    3
#define SN_result_other_mining      4
#define SN_result_other_station     5

#define min(a,b)    ((a)<=(b)?(a):(b))

// ===================================================================

    // declare prototypes of ZR functions to avoid error messages
void PsendMessage(unsigned short message);

// ===================================================================

    int i, bit_array, mask, pref;

// this array allows SPHERE2's prefs to be "turned around" to look like SPHERE1's prefs
int switch_spheres[] = {1,0,3,2,5,4,7,6,9,8,11,10};

// clear the SN_2D_data array in case it was previously used
for (i = 0; i < SN_data_size; ++i)
    SN_2D_data[i] = 0;

SN_Failed = 0;

// check validity of sphere_num
if (sphere_num != 1 && sphere_num != 2)
    return 2;

// check that all valid preferences are used in the array
bit_array=0;
for (i = 0; i < SN_num_prefs; ++i) {
    pref = preferences[i];
    if (pref < 0 || pref >= SN_num_prefs)
        return 1;
    mask = 1 << preferences[i];
    if ((bit_array & mask) > 0)
        return 1;
    bit_array |= mask;
}

// store our preferences in SPHERE1 order
for (i = 0; i < SN_num_prefs; ++i) {
    pref = preferences[i];
    if (sphere_num == 1)
        SN_2D_data[SN_my_prefs+i] = pref;
    else
        SN_2D_data[SN_my_prefs+i] = switch_spheres[pref];
}


// store time, step_num, sphere_num
SN_2D_data[SN_start_time] = time;
SN_2D_data[SN_sphere_num] = sphere_num;
SN_2D_data[SN_step_num] = 0;

// send intention to use SN Protocol to partner
PsendMessage(SN_want_to_use_2D_protocol);

return 0;

//END::PROC::SN_2D_Setup
}
static int SN_2D_Proceed (float *myState, float *otherState, float time)
{
//BEGIN::PROC::SN_2D_Proceed
// ====================== # defines ================================
// The #defines used by all SN_2D routines
#define SN_data_size    60      // this must be at least: 12 + 3*SN_num_prefs
#define SN_num_prefs    12
#define SN_want_to_use_2D_protocol     1729

// steps through the algorithm
#define SN_step_check_incoming_agreement     1
#define SN_step_start_sending_prefs		    1
#define SN_step_start_receiving_prefs		2
        // recalculate below if SN_num_prefs changes
#define SN_step_end_sending_prefs		    5  
#define SN_step_end_receiving_prefs         6
#define SN_step_conflict_resolution		    6
#define SN_step_final_processing		    7

// positions inside the SN_2D_data array
#define SN_step_num     0
#define SN_start_time   1
#define SN_sphere_num   2
#define SN_my_asteroid  3
#define SN_my_mining    4
#define SN_my_station   5
#define SN_other_asteroid   6
#define SN_other_mining     7
#define SN_other_station    8
#define SN_num_ties         9
#define SN_randnum_for_station    10
#define SN_randnum_for_tie        11
#define SN_my_prefs     12
        // recalculate below if SN_num_prefs changes
#define SN_other_prefs  24
#define SN_ties         36

#define SN_result_my_asteroid   0
#define SN_result_my_mining     1
#define SN_result_my_station    2
#define SN_result_other_asteroid    3
#define SN_result_other_mining      4
#define SN_result_other_station     5

#define min(a,b)    ((a)<=(b)?(a):(b))

// ===================================================================

// ===================================================================
// This routine does all of the major processing, SPHERE-SPHERE communications,
//   analysis, conflict resolution, etc.
int incoming, starting_pref, nprefs, i, mask, shift, one_pref, outgoing, bit_array;
int num_ties, highest_rank, random_int, sphere_num, highest_pref;
int my_asteroid, my_mining, other_asteroid, other_mining;

unsigned char joint_ranks[SN_num_prefs];
float random_sum = 0.0;

int step_num = SN_2D_data[SN_step_num];
int calculated_step = time - SN_2D_data[SN_start_time];

// this array allows SPHERE2's prefs to be "turned around" to look like SPHERE1's prefs
int switch_spheres[] = {1,0,3,2,5,4,7,6,9,8,11,10};

    // declare prototypes of ZR functions to avoid error messages
void PsendMessage(unsigned short message);
unsigned short PgetMessage();

if (SN_Failed != 0)
	return SN_Failed;

if (calculated_step == step_num)
	return 0;	// routine was called more than once within the same time step, so ignore

SN_2D_data[SN_step_num] = ++step_num;

// check if partner agrees to use SN Protocol
if (step_num == SN_step_check_incoming_agreement) {
	incoming = PgetMessage();
	if (incoming != SN_want_to_use_2D_protocol) 
		return 2;
}

// are we expecting other's prefs?
if (step_num >= SN_step_start_receiving_prefs && step_num <= SN_step_end_receiving_prefs) {
    incoming = PgetMessage();
    if (incoming == 0) // they did not send anything
        return 1;

    // unpack and store the incoming 4 (or fewer) prefs
    starting_pref = 4 * (step_num - SN_step_start_receiving_prefs);
    nprefs = min(4,(SN_num_prefs-starting_pref));
    mask = 15;
    shift = 1;
    for (i = 0; i < nprefs; ++i) {
        one_pref = (incoming / shift) & mask;
        SN_2D_data[SN_other_prefs+starting_pref+i] = one_pref;
        shift *= 16;
    }
}
    
// are we sending prefs?  then pack and send 4 (or fewer) of our prefs
if (step_num >= SN_step_start_sending_prefs && step_num <= SN_step_end_sending_prefs) {
    starting_pref = 4 * (step_num - SN_step_start_sending_prefs);
    nprefs = min(4,(SN_num_prefs-starting_pref));
    shift = 1;
    outgoing = 0;
    for (i = 0; i < nprefs; ++i) {
        one_pref = SN_2D_data[SN_my_prefs+starting_pref+i] * shift;
        outgoing |= one_pref;
        shift *= 16;
    }
    if (outgoing == 0)      // if the sole message is 0, change to 16
        outgoing = 16;
    // send it
    PsendMessage(outgoing);
}

// analyzing and send conflict resolution message (choice of station, and choose between ties)
if (step_num == SN_step_conflict_resolution) {
    // first phase: check if incoming prefs are unique and in range (no hanky-panky)
    bit_array = 0;
    for (i = 0; i < SN_num_prefs; ++i) {
        one_pref = SN_2D_data[SN_other_prefs+i];
        if (one_pref < 0 || one_pref >= SN_num_prefs) 
            return 1;

        mask = 1 << one_pref;
        if ((bit_array & mask) > 0) 
            return 1;

        bit_array |= mask;
    }
    
    // combine the pref lists
    for (i = 0; i < SN_num_prefs; ++i)
        joint_ranks[i] = 0;
    for (i = 0; i < SN_num_prefs; ++i) {
        one_pref = SN_2D_data[SN_my_prefs+i];
        joint_ranks[one_pref] += SN_num_prefs - i;
        one_pref = SN_2D_data[SN_other_prefs+i];
        joint_ranks[one_pref] += SN_num_prefs - i;
    }
    // find the highest rank
    highest_rank = joint_ranks[0];
    for (i = 1; i < SN_num_prefs; ++i) 
        if (joint_ranks[i] > highest_rank)
            highest_rank = joint_ranks[i];

    // copy the prefs with the highest rank into the tie area of SN_2D_data
    num_ties = 0;
    for (i = 0; i < SN_num_prefs; ++i) {
        if (joint_ranks[i] == highest_rank) {
            SN_2D_data[SN_ties+num_ties] = i;
            ++num_ties;
        }
    }
    SN_2D_data[SN_num_ties] = num_ties;

    // start random number generator
    for (i = 0; i < 12; ++i)
        random_sum += myState[i] + otherState[i];
    random_sum *= 1000;
    // select random number 1 or 2 for choosing station
    random_int = random_sum;
    random_int = (random_int & 1) + 1;
    SN_2D_data[SN_randnum_for_station] = random_int;
    outgoing = random_int;
    // if num_ties > 1, then choose a random number between 0 and num_ties-1
    random_int = random_sum;
    random_int = random_int % num_ties;
    outgoing |= random_int * 16;
    SN_2D_data[SN_randnum_for_tie] = random_int;
    // send conflict resolution random number(s) to partner
    PsendMessage(outgoing);
}

// receive conflict resolution message, and set final choices
if (step_num == SN_step_final_processing && SN_2D_data[SN_my_station] == 0) {
    incoming = PgetMessage();
    if (incoming == 0) 
        return 1;

    sphere_num = SN_2D_data[SN_sphere_num];

    // choose mining station
    random_int = (incoming & 15) + SN_2D_data[SN_randnum_for_station] + sphere_num;
    random_int = (random_int & 1) + 1;
    if (sphere_num == 1) {
        SN_2D_data[SN_my_station] = random_int;
        SN_2D_data[SN_other_station] = 3 - random_int;
    }
    else {
        SN_2D_data[SN_my_station] = 3 - random_int;
        SN_2D_data[SN_other_station] = random_int;
    }

    // if num_ties > 1 then choose among highest ranking prefs
    num_ties = SN_2D_data[SN_num_ties];
    if (num_ties > 1) {
        random_int = ((incoming / 16) + SN_2D_data[SN_randnum_for_tie]) % num_ties;
        highest_pref = SN_2D_data[SN_ties + random_int];
    }
    else
        highest_pref = SN_2D_data[SN_ties];
    
    // switch prefs order if we're SPHERE2 (undoing what we did in SN_2D_Setup())
    if (sphere_num == 2)
        highest_pref = switch_spheres[highest_pref];
    
    // decode the result
    if (highest_pref == 0) {
        my_asteroid = 1;    // Opulens
        my_mining = 1;      // orbit
        other_asteroid = 1; // Opulens
        other_mining = 2;   // spin
    } else if (highest_pref == 1) {
        my_asteroid = 1;    // Opulens
        my_mining = 2;      // spin
        other_asteroid = 1; // Opulens
        other_mining = 1;   // orbit
    } else if (highest_pref == 2) {
        my_asteroid = 2;    // Indigens
        my_mining = 1;      // orbit
        other_asteroid = 2; // Indigens
        other_mining = 2;   // spin
    } else if (highest_pref == 3) {
        my_asteroid = 2;    // Indigens
        my_mining = 2;      // spin
        other_asteroid = 2; // Indigens
        other_mining = 1;   // orbit
    } else if (highest_pref == 4) {
        my_asteroid = 1;    // Opulens
        my_mining = 1;      // orbit
        other_asteroid = 2; // Indigens
        other_mining = 1;   // orbit
    } else if (highest_pref == 5) {
        my_asteroid = 2;    // Indigens
        my_mining = 1;      // orbit
        other_asteroid = 1; // Opulens
        other_mining = 1;   // orbit
    } else if (highest_pref == 6) {
        my_asteroid = 1;    // Opulens
        my_mining = 1;      // orbit
        other_asteroid = 2; // Indigens
        other_mining = 2;   // spin
    } else if (highest_pref == 7) {
        my_asteroid = 2;    // Indigens
        my_mining = 2;      // spin
        other_asteroid = 1; // Opulens
        other_mining = 1;   // orbit
    } else if (highest_pref == 8) {
        my_asteroid = 1;    // Opulens
        my_mining = 2;      // spin
        other_asteroid = 2; // Indigens
        other_mining = 1;   // orbit
    } else if (highest_pref == 9) {
        my_asteroid = 2;    // Indigens
        my_mining = 1;      // orbit
        other_asteroid = 1; // Opulens
        other_mining = 2;   // spin
    } else if (highest_pref == 10) {
        my_asteroid = 1;    // Opulens
        my_mining = 2;      // spin
        other_asteroid = 2; // Indigens
        other_mining = 2;   // spin
    } else if (highest_pref == 11) {
        my_asteroid = 2;    // Indigens
        my_mining = 2;      // spin
        other_asteroid = 1; // Opulens
        other_mining = 2;   // spin
    } else 
        return 1;

    SN_2D_data[SN_my_asteroid] = my_asteroid;
    SN_2D_data[SN_my_mining] = my_mining;
    SN_2D_data[SN_other_asteroid] = other_asteroid;
    SN_2D_data[SN_other_mining] = other_mining;
}

return 0;

//END::PROC::SN_2D_Proceed
}
static int SN_2D_Result (int which)
{
//BEGIN::PROC::SN_2D_Result
// ====================== # defines ================================
// The #defines used by all SN_2D routines
#define SN_data_size    60      // this must be at least: 12 + 3*SN_num_prefs
#define SN_num_prefs    12
#define SN_want_to_use_2D_protocol     1729

// steps through the algorithm
#define SN_step_check_incoming_agreement     1
#define SN_step_start_sending_prefs		    1
#define SN_step_start_receiving_prefs		2
        // recalculate below if SN_num_prefs changes
#define SN_step_end_sending_prefs		    5  
#define SN_step_end_receiving_prefs         6
#define SN_step_conflict_resolution		    6
#define SN_step_final_processing		    7

// positions inside the SN_2D_data array
#define SN_step_num     0
#define SN_start_time   1
#define SN_sphere_num   2
#define SN_my_asteroid  3
#define SN_my_mining    4
#define SN_my_station   5
#define SN_other_asteroid   6
#define SN_other_mining     7
#define SN_other_station    8
#define SN_num_ties         9
#define SN_randnum_for_station    10
#define SN_randnum_for_tie        11
#define SN_my_prefs     12
        // recalculate below if SN_num_prefs changes
#define SN_other_prefs  24
#define SN_ties         36

#define SN_result_my_asteroid   0
#define SN_result_my_mining     1
#define SN_result_my_station    2
#define SN_result_other_asteroid    3
#define SN_result_other_mining      4
#define SN_result_other_station     5

#define min(a,b)    ((a)<=(b)?(a):(b))

// ===================================================================

// ===================================================================

    switch (which) {
    case SN_result_my_asteroid:     return SN_2D_data[SN_my_asteroid];
    case SN_result_my_mining:       return SN_2D_data[SN_my_mining];
    case SN_result_my_station:      return SN_2D_data[SN_my_station];
    case SN_result_other_asteroid:  return SN_2D_data[SN_other_asteroid];
    case SN_result_other_mining:    return SN_2D_data[SN_other_mining];
    case SN_result_other_station:   return SN_2D_data[SN_other_station];
    default:                      return -1;
}

//END::PROC::SN_2D_Result
}
