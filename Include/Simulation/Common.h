#ifndef SIMULATION_COMMON_H
#define SIMULATION_COMMON_H

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
/* grid size for simulation
 * N*N
*/
const int N = 200;
/* diffusion constant
*/
const float dDiff = 0;
const float vDiff = 0;
/* time step
*/
const float dt = 0.1;
#endif /* SIMULATION_COMMON_H
*/