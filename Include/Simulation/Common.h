#ifndef SIMULATION_COMMON_H
#define SIMULATION_COMMON_H

/* Choosing the attribute to apply the methods on
*/
typedef enum{
    DENSITY, 
    VELOCITY
}attribute;

/* Other constants that are use in this
 * application
*/
/* number of iterations in the iter solver
*/
const int kIter = 20;
/* grid size for simulation
*/
const int N = 20;
/* time step
*/
const int dt = 0.1;
#endif /* SIMULATION_COMMON_H
*/