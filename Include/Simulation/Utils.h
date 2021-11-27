#ifndef SIMULATION_UTILS_H
#define SIMULATION_UTILS_H

#include "../Simulation/Common.h"
/* Utility functions that will be used with
 * the fluid class
*/

/* swap pointers, this is mainly used to swap
 * between current and next values for density
 * and velocity
*/
void swap(float **p1, float **p2);
/* get grid position given the index
 * positions (i, j)
 * Usage: if i = 9, j = 9 in a 10x10 grid,
 * index will be 9 + (9 * 10) = 99
*/
int getIdx(int i, int j);
/* Iterative solver using Gauss_Seidel method 
 * 4x - 2y + z = -2
 * 3x + 6y - 2z = 49
 * -x -3y + 5z = -31 
 * 
 * Rearrange it to 
 * x = ... equation #1
 * y = ... equation #2
 * z = ... equation #2
 * 
 * Start with a random value (say 0) for all
 * the unknown variables, this will yield some
 * value for the unknowns. 
 * For eg: use y = z = 0 and get a value for x
 * in equation #1, use this value of x in the 
 * next equation and so on.
 * Keep doing this or iterate over a set number 
 * of times, each time using the newest updated values,
 * the solution will converge to the true solution
 * for the system of equations.
 * 
 * NOTE: This method works only if the matrix made up
 * of the coefficients are diagonally dominant
 * 
 * eg:
 *  4   -2  1
 * 3    6   -2
 * -1   -3  5
 * 
 * here, 4 > (-2) + 1, same for the rest, making
 * it diagonally dominant
 * In our equations, the denominator > sum of
 * the coefficients 
 * x = (-2 + 2y - z)/4
 * 
 * We will be solving this equation by iteration over
 * numIter times
 * 
 * next = (curr + k(sNext))/(1 + 4k)
*/
void iterSolve(attribute atType, float *curr, float *next, float k, int numIter);
/* Boundaries in the grid
 * We assume that the fluid is contained in a
 * box with solid walls: no flow should exit the walls. 
 * This simply means that the horizontal component of the 
 * velocity should be zero on the vertical walls, while the 
 * vertical component of the velocity should be zero on the 
 * horizontal walls. For the density and other attributes considered
 * in the code we simply assume continuity.
 * 
 * -------------------------
 * |  * |  v | v  | v  |  * |
 * --------------------------
 * |  > |    |    |    |  < |   In case of velocity attribute, 
 * --------------------------   '<', '>', '^', 'v' will mean a vector
 * |  > |    |    |    |  < |   opposite to that of the next cell
 * --------------------------   
 * |  > |    |    |    |  < |   For density attribute, 
 * --------------------------   they will mean the same magnitude as the
 * |  > |    |    |    |  < |   next cell, since we assume continuity
 * --------------------------   for every attribute except veclocity
 * |  * |  ^ |  ^ |  ^ |  * |
 * --------------------------   The corner cells with '*' means
 *                              0.5 * (2 nearest cells)
*/
void setBoundaries(attribute atType, float *curr);
#endif /* SIMULATION_UTILS_H
*/
