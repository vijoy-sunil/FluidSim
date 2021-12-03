#include "../../Include/Simulation/Fluid.h"
#include "../../Include/Simulation/Utils.h"
#include <stdlib.h> /* for malloc, calloc, free
*/

FluidClass::FluidClass(int _N, float _dDiff, float _vDiff, float _dt){
    N = _N;
    /* (N+2) has to be an even number, for placement
     * on the render screen
    */
    assert((N+2) % 2 == 0);
    dDiff = _dDiff;
    vDiff = _vDiff;
    dt = _dt;
    totalCells = N * N;

    dCurr = (float*)calloc(totalCells, sizeof(float));
    dNext = (float*)calloc(totalCells, sizeof(float));

    vXCurr = (float*)calloc(totalCells, sizeof(float));
    vXNext = (float*)calloc(totalCells, sizeof(float));
    vYCurr = (float*)calloc(totalCells, sizeof(float));
    vYNext = (float*)calloc(totalCells, sizeof(float));
}

FluidClass::~FluidClass(void){
    free(dCurr);
    free(dNext);

    free(vXCurr);
    free(vXNext);
    free(vYCurr);
    free(vYNext);
}

void FluidClass::addDensitySource(int i, int j, float amount){
    /* add new source to (i,j) cell, think
     * of it as adding a dye to help visulaize
     * the flow
    */
    dNext[getIdx(i, j)] = dCurr[getIdx(i, j)] + amount;
}

void FluidClass::addVelocitySource(int i, int j, float amountX, float amountY){
    /* add new source to (i,j) cell, think of it
     * as adding a wind source to change the
     * velocity vector field
    */
    vXNext[getIdx(i, j)] = vXCurr[getIdx(i, j)] + amountX;
    vYNext[getIdx(i, j)] = vYCurr[getIdx(i, j)] + amountY;
}

void FluidClass::diffuse(attribute atType){
    float k, diff;
    float *curr, *next;

    if(atType == DENSITY){
        diff = dDiff;
        curr = dCurr;
        next = dNext;
    }
    else if(atType == VELOCITY_X){
        diff = vDiff;
        curr = vXCurr;
        next = vXCurr;
    }
    else if(atType == VELOCITY_Y){
        diff = vDiff;
        curr = vYCurr;
        next = vYNext;
    }

    k = dt * diff * (N-2) * (N-2);
    iterSolve(atType, curr, next, k, kIter);
}

void FluidClass::advection(attribute atType){
    float *curr, *next;
    float dT = dt * (N-2);

    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            /* do back dT to see where the density is coming
             * from
            */
            float fX = i - (dT * vXCurr[getIdx(i, j)]);
            float fY = j - (dT * vYCurr[getIdx(i, j)]);
            /* limit boundaries
             * TODO: double check this
            */
            fX = (fX < 0.5) ? 0.5 : fX;
            fY = (fY < 0.5) ? 0.5 : fY;
            fX = (fX > (N-2) + 0.5) ? (N-2) + 0.5 : fX;
            fY = (fY > (N-2) + 0.5) ? (N-2) + 0.5 : fY;
            /* get surrounding cell coordinates
            */
            float i0 = (int)fX;
            float i1 = i0 + 1;
            float j0 = (int)fY;
            float j1 = j0 + 1;
            /* get distance to cell centers
            */
            float s1 = fX - i0;
            float s0 = 1 - s1;
            float t1 = fY - j0;
            float t0 = 1 - t1;
            /* interpolate
            */
            if(atType == DENSITY){
                curr = dCurr;
                next = dNext;
            }
            else if(atType == VELOCITY_X){
                curr = vXCurr;
                next = vXNext;
            }
            else if(atType == VELOCITY_Y){
                curr = vYCurr;
                next = vXNext;
            }

            float z0 = t0 * (curr[getIdx(i0, j0)]) + t1 * (curr[getIdx(i0, j1)]);
            float z1 = t0 * (curr[getIdx(i1, j0)]) + t1 * (curr[getIdx(i1, j1)]);
            next[getIdx(i,j)] = s0 * z0 + s1 * z1;
        }
    }
    setBoundaries(atType, curr);
}

void FluidClass::clearDivergence(void){
    /* you could reuse the next pointer to store
     * divergence and p, or use a new one
    */
    float *div = (float*)malloc(sizeof(float) * N * N);
    float *p = (float*)malloc(sizeof(float) * N * N);
    float dist = 1/N;

    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            /* divergence
            */
            div[getIdx(i, j)] = -0.5 * dist * 
                                (vXCurr[getIdx(i+1, j)] - vXCurr[getIdx(i-1, j)] + 
                                 vYCurr[getIdx(i, j+1)] - vYCurr[getIdx(i, j-1)]);
            p[getIdx(i,j)] = 0;
        }
    }
    setBoundaries(OTHER, div);
    setBoundaries(OTHER, p);

    iterSolve(OTHER, div, p, 1, kIter);

    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            vXCurr[getIdx(i, j)] -= 0.5 * N * (p[getIdx(i+1, j)] - p[getIdx(i-1, j)]);
            vYCurr[getIdx(i, j)] -= 0.5 * N * (p[getIdx(i, j+1)] - p[getIdx(i, j-1)]); 
        }
    }
    setBoundaries(VELOCITY_X, vXCurr);
    setBoundaries(VELOCITY_Y, vYCurr);

    free(div);
    free(p);
}

void FluidClass::densityStep(void){
    /* adding source will be done as an
     * input, so it is not included in this
     * routine
    */
    diffuse(DENSITY);
    swap(&dNext, &dCurr);

    advection(DENSITY);
    swap(&dNext, &dCurr);
    /* no you can render out dCurr
    */
}

void FluidClass::velocityStep(void){
    /* adding source will be done as an
     * input, so it is not included in this
     * routine
    */
    diffuse(VELOCITY_X);
    swap(&vXNext, &vXNext); 

    diffuse(VELOCITY_Y);
    swap(&vYNext, &vYCurr);

    clearDivergence();

    advection(VELOCITY_X);
    swap(&vXNext, &vXNext); 

    advection(VELOCITY_Y);
    swap(&vYNext, &vYCurr);

    clearDivergence();
}

void FluidClass::simulationStep(void){
    velocityStep();
    densityStep();
}