#ifndef CONTROL_CONSTANTS_H
#define CONTROL_CONSTANTS_H

/* number of iterations in the iter solver
*/
const int kIter = 20;
/* diffusion constant
*/
const float dDiff = 0.0;
const float vDiff = 0.000001;
/* grid size for simulation N*N
 * NOTE: N+2 has to be an even number
*/
const int N = 128;
/* time step
*/
const float dt = 0.2;
/* scale render window size, changing this factor resizes 
 * everything inside the window and no special calculation
 * is necessary
*/
const int scale = 4;
#endif /* CONTROL_CONSTANTS_H
*/