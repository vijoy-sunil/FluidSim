#include "../../Include/Simulation/Fluid.h"
#include "../../Include/Control/Constants.h"
#include "../../Include/Control/Utils.h"
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
    dPrev = (float*)calloc(totalCells, sizeof(float));

    vXCurr = (float*)calloc(totalCells, sizeof(float));
    vXPrev = (float*)calloc(totalCells, sizeof(float));
    vYCurr = (float*)calloc(totalCells, sizeof(float));
    vYPrev = (float*)calloc(totalCells, sizeof(float));
}

FluidClass::~FluidClass(void){
    free(dCurr);
    free(dPrev);

    free(vXCurr);
    free(vXPrev);
    free(vYCurr);
    free(vYPrev);
}

void FluidClass::addDensitySource(int i, int j, float amount){
    /* add new source to (i,j) cell, think
     * of it as adding a dye to help visulaize
     * the flow
    */
    dPrev[getIdx(i, j)] += amount;
}

void FluidClass::addVelocitySource(int i, int j, float amountX, float amountY){
    /* add new source to (i,j) cell, think of it
     * as adding a wind source to change the
     * velocity vector field
    */
    vXCurr[getIdx(i, j)] += amountX;
    vYCurr[getIdx(i, j)] += amountY;
}

void FluidClass::diffuse(attribute atType, float *curr, float *prev, float diff){
    float k = dt * diff * (N-2) * (N-2);
    iterSolve(atType, curr, prev, k, kIter);
}

void FluidClass::advection(attribute atType, float *curr, float *prev, float *vX, float *vY){
    float dT = dt * (N-2);

    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            /* do back dT to see where the density is coming
             * from
            */
            float fX = i - (dT * vX[getIdx(i, j)]);
            float fY = j - (dT * vY[getIdx(i, j)]);
            /* limit boundaries
            */
            fX = (fX < 0.5) ? 0.5 : fX;
            fY = (fY < 0.5) ? 0.5 : fY;
            fX = (fX > (N-2) + 0.5) ? (N-2) + 0.5 : fX;
            fY = (fY > (N-2) + 0.5) ? (N-2) + 0.5 : fY;
            /* get surrounding cell coordinates
            */
            int i0 = (int)fX;
            int i1 = i0 + 1;
            int j0 = (int)fY;
            int j1 = j0 + 1;
            /* get distance to cell centers
            */
            float s1 = fX - i0;
            float s0 = 1.0 - s1;
            float t1 = fY - j0;
            float t0 = 1.0 - t1;
            /* interpolate
            */
            float z0 = t0 * (prev[getIdx(i0, j0)]) + t1 * (prev[getIdx(i0, j1)]);
            float z1 = t0 * (prev[getIdx(i1, j0)]) + t1 * (prev[getIdx(i1, j1)]);
            curr[getIdx(i,j)] = (s0 * z0) + (s1 * z1);
        }
    }
    setBoundaries(atType, curr);
}

void FluidClass::clearDivergence(float *vX, float *vY, float *div, float *p){
    /* we reuse the already allocated memory to store
     * div and p values
    */
    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            /* divergence
            */
            div[getIdx(i, j)] = -0.5 * 
                                (vX[getIdx(i+1, j)] - vX[getIdx(i-1, j)] + 
                                 vY[getIdx(i, j+1)] - vY[getIdx(i, j-1)])/N;
            p[getIdx(i,j)] = 0;
        }
    }
    setBoundaries(CLEAR_DIVERGENCE, div);
    setBoundaries(CLEAR_DIVERGENCE, p);

    iterSolve(CLEAR_DIVERGENCE, p, div, 1, kIter);

    for(int i = 1; i < N-1; i++){
        for(int j = 1; j < N-1; j++){
            vX[getIdx(i, j)] -= 0.5 * N * (p[getIdx(i+1, j)] - p[getIdx(i-1, j)]);
            vY[getIdx(i, j)] -= 0.5 * N * (p[getIdx(i, j+1)] - p[getIdx(i, j-1)]); 
        }
    }
    setBoundaries(VELOCITY_X, vX);
    setBoundaries(VELOCITY_Y, vY);
}

void FluidClass::densityStep(void){
    /* adding source will be done as an input, so 
     * it is not included in this routine
    */
    
    /* We reach here after adding source, meaning
     * we have our starting values stored in dPrev
    */
    diffuse(DENSITY, dCurr, dPrev, dDiff);
    /* After diffusion, we have the results store in
     * dCurr
    */
    advection(DENSITY, dPrev, dCurr, vXCurr, vYCurr);
    /* After advection, the new values will be written to
     * dPrev using the diffusion result that was stored in 
     * dCurr.
    */
    
    /* In the render loop, we can render out dPrev which
     * has the advection result. In the next time step,
     * we operate on the dPrev thus forming a cycle
    */
}

void FluidClass::velocityStep(void){
    /* adding source will be done as an input, so it 
     * is not included in this routine
    */

    /* We reach here after adding source, meaning
     * we have our starting values stored in vXCurr
     * and vYCurr
    */
    diffuse(VELOCITY_X, vXPrev, vXCurr, vDiff);
    diffuse(VELOCITY_Y, vYPrev, vYCurr, vDiff);
    /* after diffusion, vXPrev and vYPrev will have the
     * result
    */
    clearDivergence(vXPrev, vYPrev, vXCurr, vYCurr);
    /* After clearDivergence(), we have our results in vXPrev
     * and vYPrev
    */
    advection(VELOCITY_X, vXCurr, vXPrev, vXPrev, vYPrev);
    advection(VELOCITY_Y, vYCurr, vYPrev, vXPrev, vYPrev);
    /* After advection, the results will be in vXCurr and
     * vYCurr
    */
    clearDivergence(vXCurr, vYCurr, vXPrev, vYPrev);
}

void FluidClass::simulationStep(void){
    velocityStep();
    densityStep();
}

void FluidClass::iterSolve(attribute atType, float *curr, float *prev, float k, int numIter){
    if(curr == NULL || prev == NULL)
        assert(false);
    
    /* 1 is passed in when iterSolve is called
     * to solve p vector field
    */
    float denom = (atType == CLEAR_DIVERGENCE) ? 4 : (1 + 4 * k);
    while(numIter != 0){
        /* process all grid cells except the
         * border walls
        */
        for(int i = 1; i < N-1; i++){
            for(int j = 1; j < N-1; j++){
                float s = curr[getIdx(i-1, j)] + 
                          curr[getIdx(i+1, j)] +
                          curr[getIdx(i, j-1)] +
                          curr[getIdx(i, j+1)];

                curr[getIdx(i, j)] = (prev[getIdx(i, j)] + (k * s))/denom;    
            }
        }
        /* process border grid cells
        */
        setBoundaries(atType, curr);
        numIter--;
    }
}

void FluidClass::setBoundaries(attribute atType, float *arr){
    if(arr == NULL)
        assert(false);
        
    /* the vertical (Y) componenet of velocity should be
     * negated at the top and bottom border cells except
     * the corner cells. The x component and the density
     * will be the same as the previous cell
    */
    for(int i = 1; i < N-1; i++){
        arr[getIdx(i, 0)] = 
            atType == VELOCITY_Y ? -arr[getIdx(i, 1)] : arr[getIdx(i, 1)];
        arr[getIdx(i, N-1)] = 
            atType == VELOCITY_Y ? -arr[getIdx(i, N-2)] : arr[getIdx(i, N-2)];
    }
    /* the horizontal component (X) of velocity should be
     * negated at the left and right border cells except the
     * corner cells
    */
    for(int j = 1; j < N-1; j++){
        arr[getIdx(0, j)] = 
            atType == VELOCITY_X ? -arr[getIdx(1, j)] : arr[getIdx(1, j)];
        arr[getIdx(N-1, j)] = 
            atType == VELOCITY_X ? -arr[getIdx(N-2, j)] : arr[getIdx(N-2, j)];
    }

    /* corner cells
    */
    arr[getIdx(0, 0)] = 0.5 * (arr[getIdx(1, 0)] + arr[getIdx(0, 1)]);
    arr[getIdx(N-1, 0)] = 0.5 * (arr[getIdx(N-2, 0)] + arr[getIdx(N-1, 1)]);
    arr[getIdx(0, N-1)] = 0.5 * (arr[getIdx(1, N-1)] + arr[getIdx(0, N-2)]);
    arr[getIdx(N-1, N-1)] = 0.5 * (arr[getIdx(N-2, N-1)] + arr[getIdx(N-1, N-2)]);
}