#ifndef SIMULATION_COMMON_H
#define SIMULATION_COMMON_H

#include <cassert> /* for assert
*/
/* Choosing the attribute to apply the methods on
*/
typedef enum{
    DENSITY, 
    VELOCITY_X, 
    VELOCITY_Y, 
    OTHER
}attribute;

/* number of iterations in the iter solver
*/
const int kIter = 20;
/* grid size for simulation N*N
 * NOTE: N+2 has to be an even number
*/
const int N = 50;
/* diffusion constant
*/
const float dDiff = 0;
const float vDiff = 0;
/* time step
*/
const float dt = 0.1;
#endif /* SIMULATION_COMMON_H
*/