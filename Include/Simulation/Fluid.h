#ifndef SIMULATION_FLUID_H
#define SIMULATION_FLUID_H

/* Choose attribute to run sim function on
*/
typedef enum{
    DENSITY,
    VELOCITY_X, 
    VELOCITY_Y,
    CLEAR_DIVERGENCE
}attribute;

/* the 2D fluid class based on Navier-Stokes equations
 * for incompressible fluids
 *
 * There are 2 equations:
 * (1) divergence of verlocity = 0
 * What this means is that velocities in neighboring
 * areas of a fluid cannot be flowing towards each other
 * or away from each other. This is because if it is possible
 * then somewhere  matter would have to created
 * out of nothing or disappear into nothing.
 *      ^
 *      |
 *  <---*----> flowing away/ matter created out of nothing
 *      |
 *      v
 * 
 * Essentially, the equation makes sure that mass is conserved
 * in the fluid. (CONSERVATION OF MASS)
 * 
 * (2) Acceleration of a fluid depends upon its internal forces
 * (pressure gradient (fluid flows from high to low pressure) & 
 * viscocity) and external forces (gravity, wind, walls etc.)
 * 
 * +------------------------------------------------------------------------+
 * |    i.e The first term says that the density should follow the          |
 * |    velocity field, the second states that the density may diffuse      | 
 * |    at a certain rate and the third term says that the density          |
 * |    increases due to sources                                            |
 * |    ADVECTION + DIFFUSION + SOURCES                                     |
 * +------------------------------------------------------------------------+
*/
class FluidClass{
    private:
        int totalCells;
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
         * curr = (prev + k(sCurr))/(1 + 4k)
        */
        void iterSolve(attribute atType, float *curr, float *prev, float k, int numIter);
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
        void setBoundaries(attribute atType, float *arr);
    public:
        /* Fluid representaion based on a grid with
         * stationary regions (NxN regions), with 
         * attributes - density and velocity that
         * represent the average of all the imaginary
         * particles that would be in each grid cell
         * 
         * NOTE: The border walls are included within
         * NxN grid, so we would be working on 
         * N-1 x N-1 grid 
        */
        int N;
        /* this is the time step resolution
         * In the simulation, we take snapshot of
         * all the attributes in a given time, then
         * calculate how those attributes will change
         * over many time steps
         * 
         * NOTE: these attributes can also be continuously
         * altered throughout the simulation using external
         * forces as an interactive input
        */
        float dt;
        /* the amount of diffusion, we calculate diffusion rate
         * using this quantity.
         * Here, we will have diffusion of density and velocity.
        */
        float dDiff, vDiff;
        /* current densitiy and prev density for all grid cells
        */
        float *dCurr, *dPrev;
        /* current velocity and previous velocity
         * for all grid cells in x and y axis
         *
         * The velocity attribute tells us how
         * fast the fluid is moving and in what
         * direction
         * 
         * NOTE: the way that all attributes 
         * disrtibute within the simulation area
         * depends on the velocity, even the velocity
         * attribute itself (self advection)
        */
        float *vXCurr, *vXPrev;
        float *vYCurr, *vYPrev;

        /* constructor takes in N (NxN will be grid size), 
         * time step dt (how big each step is), rates of
         * diffusion - density diffusion and viscous diffusion
        */
        FluidClass(int _N, float _dDiff, float _vDiff, float _dt);
        /* destructor needs to free all dynamic memory allocated
        */
        ~FluidClass(void);
        /* The solver will sove the 3 terms that appear in the
         * equation in the reverse order. So, the first one
         * is adding source
        */
        void addDensitySource(int i, int j, float amount);
        void addVelocitySource(int i, int j, float amountX, float amountY);
        /* The second is diffusion
         *
         * diffuse function which is used for density
         * and velocity (viscous diffusion)
         * 
         * As an example, let us do density diffusion.
         * The cell’s density will decrease by losing density 
         * to its neighbors, but will also increase due to 
         * densities flowing in from the neighbors, which results 
         * in a net difference of
         * 
         * d(i-1,j)) + d(i+1,j) + d(i,j-1) + d(i,j+1) - 4 * d(i,j)     
         * Let this be equal to s
         * 
         * A possible implementation of a diffusion solver then 
         * simply computes these exchanges at every grid cell and 
         * adds them to the existing values.
         * 
         * dCurr(i,j) = dPrev(i,j) + k * (sPrev);
         * This is a linear interpolation. But, when k > 1
         * we overshoot the target value. This is 
         * similar to overshooting in control system. If we 
         * add too much correction we overshoot. This will
         * result in unstable changes in density values such
         * as going negative, and jumping up and down unexpectedly
         * (rippling effect)
         * 
         * A better solution than capping k to max value of 1 is
         * to find the curr value which when rewinded back in time
         * results in the prev value
         * i.e dPrev(i,j) = dCurr(i,j) - k * (sCurr)
         * 
         * Rearranging this,
         * dCurr(i,j) = (dPrev(i,j) + k * sCurr)/(1 + 4 * k)
         * We have turned the linear relation between dNext
         * and k to a hyperbolic one. So instead of overshooting
         * with changes in k, we tend to converge towards the target
         * value. This is a stable way of interpolating.
         * 
         * But, sCurr is unknown in this equation.
         * sCurr = dCurr(i+1, j) + dCurr(i-1, j) + dCurr(i, j+1) + 
         * dCurr(i, j-1)
         * 
         * In short, we are trying to find dCurr using the surrounding 
         * dCurr values. This is a system of simultaneous equations and 
         * we solve this using Gauss-Seidel method where we approximate
         * the solution using an iterative solver.
         * 
         * NOTE: our density equation satisfies the constraint
         * for the Gauss-Seidel method; diagonal dominant coefficient
         * matrix
         * 
         * At the end of several iterations, the attribute (here it is
         * density) will converge to the diffused densities, i.e we will
         * have solved for dNext
        */
        void diffuse(attribute atType, float *curr, float *prev, float diff);
        /* The third and final term is advection
         * Advection is where the attribute follows the velocity field,
         * denisty and velocity itself
         * Let us take density advection here as an example,
         * The calculation for the new density at i,j after
         * advection would be much simpler if the velocity vector
         * points to the center of a grid cell. In such a case, we
         * could just add the density to the cell's current density.
         * 
         * But the vector never points to the center of the cell, 
         * instead it points to somewhere between the 4 grid cells
         *  _____________
         *  |  x  |  x  |    Let x, be the center of 4 grid cells.
         *  ------o------    
         *  | x   |  x  |
         *  --------------------  Let the velocity vector at this
         *               |  x  |  grid cell point at 'o'. Then, the
         *               -------  the density that moves along this 
         *                        vector will affect all 4 of these
         *                        grid cells surrounding the target
         *                        spot.
         * 
         * It is difficult to ditribute this density value to the
         * 4 cells, moreover we will have to do the same distribution
         * since there will be more than one velocity vector pointing
         * at or around 'o'
         * 
         * An easier way to distribute the density proportionately 
         * to all 4 cells would be to look at a grid cell, and trace
         * backwards to find where its curr density will come from
         * using its current veclocity vector. 
         * 
         * This way we will only need one calculation per cell.
         * 
         * ---------------        if we trace back using the velocity
         * |  x   |      |        field, the target spot would be at 'o'
         *  --------------------  
         * |      |   a  |  b  |  Next, we will linear interpolate between
         * --------------o------  the 4 surrounding cells to get the target
         *        |   c  |  d  |  density value
         *        --------------
         * Let the left bottom grid be (12, 0)
         * x cell will then be (12, 2)
         * Let the velocity vector be vX = -1.5, vY = 1.5
         * 
         * To find the position of 'o', lets call it f
         * fX = 12 + dt(1.5) = 13.5
         * fY = 2 - dt(1.5) = 0.5
         * 
         * Get the position of the 4 surrounding cells
         * (i0,j0), (i1,j0), (i0,j1), (i1,j1)
         * i0 = (int)fX = 13
         * i1 = i0 + 1 = 14
         * 
         * j0 = (int)fY = 0
         * j1 = j0 + 1 = 1
         * 
         * Now, find the distance between f and all 4 grid cells
         * s1 = fX - i0 = 0.5
         * s0 = 1 - s1 = 0.5
         * 
         * t1 = fY - j0 = 0.5
         * t0 = 1 - t1 = 0.5
         * 
         *      -----------------
         *   t1 |       |       |
         *      --------o--------
         *   t0 |       |       |
         *      -----------------
         *          s0      s1
         * 
         * To linear interpolate, do vertical distance first
         * t0 * (d(i0,j0)) + t1 * (d(i0,j1)) = z0
         * and,
         * t0 * (d(i1,j0)) + t1 * (d(i1,j1)) = z1
         * 
         * Once we have this, we interpolate these 2 using s0, s1
         * (s0 * z0) + (s1 * z1)
         * 
         * This will be the new density after advection
         * dNext
        */   
        void advection(attribute atType, float *curr, float *prev, float *vX, float *vY); 
        /* Clearing divergence of the vector field.
         * This is only used on the velocity attribute, so no
         * parameters are passed in.
         * Curl and Divergence are properties universal to
         * every vector field. We want our fluid's  velocity
         * field to have curl but not divergence. However, after
         * we do all that diffusion and advection to simulate the
         * change in fluid's VELOCITY field, we actually end up
         * with a velocity field which has both curl and divergence.
         * 
         * We need to extract out the divergence free part using
         * Helmholtz decomposition method. According to his
         * theorem, any vector field can be expressed as the 
         * sum of CURL FREE VECTOR FIELD + DIVERGENCE FREE VECTOR
         * FIELD.
         *  
         * We need the divergence free part, but there is no direct
         * way to compute this. Our goal is to compute the curl free
         * part and subtract it from the original velocity field to 
         * get the divergence free vector field.
         * 
         * Steps to get curl free field:
         * (1) Calculate divergence (velocity) at every grid cell (i,j)
         * div(i,j) = (difference in x velocities across 2 horizontal cells +
         *            difference in y veclocities across 2 vertical cells)/
         *            distance between the cells
         *      
         *          = -(vXCurr(i+1,j) - vXCurr(i-1,j) +
         *            vYCurr(i,j+1) - vYCurr(i,j-1))/2
         * 
         * (2) Compute a field of p values (scalar values)
         * div(i,j) = 4p(i,j) - (p(i-1,j) + p(i+1,j) + p(i,j-1) + p(i,j+1))
         * Rearranging,
         * p(i,j) = (p(i-1,j) + p(i+1,j) + p(i,j-1) + p(i,j+1) + div(i,j))/4
         * 
         * This is a system of simultaneous equations, and we use
         * Gauss-Seidel iterative solver to solve for p(i,j)
         * 
         * Now we need to find the gradient vector field of this scalar field,
         * which is
         * divP(i,j) = (p(i+1,j) - p(i-1,j))/2, (p(i,j+1) - p(i,j-1))/2
         * 
         * One of the identities of vactor calculus states that the
         * CURL OF SUCH A GRADIENT VECTOR FIELD = 0
         * The curl of the gradient is the integral of the gradient 
         * round an infinitesimal loop which is the difference 
         * in value between the beginning of the path and the 
         * end of the path. In a scalar field there can be no difference, 
         * so the curl of the gradient is zero.
         * 
         * This is the curl free part that we are looling for.
         * Subtracting this from the original velocity field will
         * give the divergence free field
         * 
         * vXCurr = vXCurr - (p(i+1,j) - p(i-1,j))/2
         * vYCurr = vYCurr - (p(i,j+1) - p(i,j-1))/2 
        */
        void clearDivergence(float *vX, float *vY, float *div, float *p);
        /* This is the density solver and the velocity solver 
         * function that we call every time step
        */           
        void densityStep(void);
        void velocityStep(void);
        /* simulation step combines both
         * of the above steps
         * (1) velocity step
         * (2) density step
        */
        void simulationStep(void);
};
#endif /* SIMULATION_FLUID_H
*/
