#include "../../Include/Simulation/Utils.h"

void swap(float **p1, float **p2){
    float *temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

int getIdx(int i, int j){
    return i + (j * N);
}

void iterSolve(attribute atType, float *curr, float *next, float k, int numIter){
    if(curr == NULL || next == NULL)
        assert(false);
    
    /* OTHER is passed in when iterSolve is called
     * to solve p vector field
    */
    float denom = atType == OTHER ? 4 : (1 + 4*k);
    while(numIter != 0){
        /* process all grid cells except the
         * border walls
        */
        for(int i = 1; i < N-1; i++){
            for(int j = 1; j < N-1; j++){
                float s = next[getIdx(i-1, j)] + 
                          next[getIdx(i+1, j)] +
                          next[getIdx(i, j-1)] +
                          next[getIdx(i, j+1)];

                next[getIdx(i, j)] = (curr[getIdx(i, j)] + (k * s))/denom;    
            }
        }
        /* process border wall grid cells
        */
        setBoundaries(atType, next);
        numIter--;
    }
}

void setBoundaries(attribute atType, float *next){
    if(next == NULL)
        assert(false);
        
    /* border cells except corners
    */
    for(int i = 1; i < N-1; i++){
        next[getIdx(i, 0)] = 
            atType == VELOCITY_Y ? -next[getIdx(i, 1)] : next[getIdx(i, 1)];
        next[getIdx(i, N-1)] = 
            atType == VELOCITY_Y ? -next[getIdx(i, N-2)] : next[getIdx(i, N-2)];
    }

    for(int j = 1; j < N-1; j++){
        next[getIdx(0, j)] = 
            atType == VELOCITY_X ? -next[getIdx(1, j)] : next[getIdx(1, j)];
        next[getIdx(N-1, j)] = 
            atType == VELOCITY_X ? -next[getIdx(N-2, j)] : next[getIdx(N-2, j)];
    }

    /* corner cells
    */
    next[getIdx(0, 0)] = 0.5 * (next[getIdx(1, 0)] + next[getIdx(0, 1)]);
    next[getIdx(N-1, 0)] = 0.5 * (next[getIdx(N-2, 0)] + next[getIdx(N-1, 1)]);
    next[getIdx(0, N-1)] = 0.5 * (next[getIdx(1, N-1)] + next[getIdx(0, N-2)]);
    next[getIdx(N-1, N-1)] = 0.5 * (next[getIdx(N-2, N-1)] + next[getIdx(N-1, N-2)]);
}