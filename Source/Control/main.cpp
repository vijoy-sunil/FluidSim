#include "../../Include/Control/Constants.h"
#include "../../Include/Simulation/Fluid.h"
#include "../../Include/Control/Utils.h"
#include "../../Include/Visualization/Shader/Shader.h"

int main(void){
    /* create fluid object
    */
    FluidClass Fluid(N, dDiff, vDiff, dt);
    /* OpenGL bringup routine
    */
    GLFWwindow* window = openGLBringUp();
    if(!window)
        return -1;
    /* Build and compile the shader program
    */
    ShaderClass Shader;
    /* create all vertices starting from bottom left
     * to top right

     * NOTE: there are a total of (N+2)*(N+2) cells
     * eventhough the fluid class see N*N cells
    */
    for(int i = 0; i < (N+2); i++){
        for(int j = 0; j < (N+2); j++){
            genCellVerticesWrapper(i, j);
            /* check if border cell
            */
            if((i == 0) || (i == (N + 2) - 1) ||
               (j == 0) || (j == (N + 2) - 1))
                genCellColor(i, j, borderR, borderG, borderB, borderAlpha);
            else    
                genCellColor(i, j, cellR, cellG, cellB, cellAlpha);
        }
    }

    moveDataToGPU(VERTEX);
    moveDataToGPU(COLOR);

    /* Right now we sent the input vertex data to the GPU 
     * and instructed the GPU how it should process the 
     * vertex data within a vertex and fragment shader. We're 
     * almost there, but not quite yet. OpenGL does not yet 
     * know how it should interpret the vertex data in memory 
     * and how it should connect the vertex data to the vertex 
     * shader's attributes.
     * 
     * The vertex shader allows us to specify any input we 
     * want in the form of vertex attributes and while 
     * this allows for great flexibility, it does mean we 
     * have to manually specify what part of our input data 
     * goes to which vertex attribute in the vertex shader. 
     * This means we have to specify how OpenGL should 
     * interpret the vertex data before rendering.
    */
    setVertexAttribute(VERTEX);
    setVertexAttribute(COLOR);

    /* We need this to enable alpha transperancy of our cells.
     * The glBlendFunc(GLenum sfactor, GLenum dfactor) function 
     * expects two parameters that set the option for the source 
     * and destination factor. 
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* We don't want the application to draw a single image 
     * and then immediately quit and close the window. We 
     * want the application to keep drawing images and handling 
     * user input until the program has been explicitly told 
     * to stop. 
     * 
     * For this reason we have to create a while loop, that we 
     * now call the RENDER LOOP, that keeps on running until we 
     * tell GLFW to stop.
     * 
     * The glfwWindowShouldClose function checks at the start 
     * of each loop iteration if GLFW has been instructed to 
     * close. If so, the function returns true and the render 
     * loop stops running, after which we can close the 
     * application.
    */
    while (!glfwWindowShouldClose(window)){
        /* We want to have some form of input control in GLFW 
         * and we can achieve this with several of GLFW's
         * input functions. We'll be using GLFW's glfwGetKey 
         * function that takes the window as input together 
         * with a key. The function returns whether this key 
         * is currently being pressed. We're creating a 
         * processInput function to keep all input code 
         * organized:
        */
        processInput(window);

        /* We want to clear the screen with a color of our 
         * choice. At the start of frame we want to clear the 
         * screen. Otherwise we would still see the results 
         * from the previous frame (this could be the effect 
         * you're looking for, but usually you don't). We can 
         * clear the screen's color buffer using glClear where 
         * we pass in buffer bits to specify which buffer we 
         * would like to clear. The possible bits we can set 
         * are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT and 
         * GL_STENCIL_BUFFER_BIT. 
         * 
         * Note that we also specify the color to clear the 
         * screen with using glClearColor. Whenever we call 
         * glClear and clear the color buffer, the entire color 
         * buffer will be filled with the color as configured
         * by glClearColor
         * 
         * The glClearColor function is a state-setting function 
         * and glClear is a state-using function in that it uses 
         * the current state to retrieve the clearing color from.
        */
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Use the shader object that we linked using our shader 
         * files
        */
        Shader.use();
        /* add source at cell selected by mouse click, at start we
         * add source at the middle by default.
         *
         * NOTE: the amount of density added is set to be randomly
         * chosen between 0.0 and 1.0 so that it is easier to set
         * the alpha term while rendering
        */
        for(int i = -1; i <= 1; i++){
            for(int j = -1; j <= 1; j++){
                Fluid.addDensitySource(cellX + i, cellY + j, getRandomAmount(0.0, 1.0));   
            }
        }
        Fluid.addVelocitySource(cellX, cellY, getRandomAmount(-1.0, 1.0), 
                                              getRandomAmount(-1.0, 1.0));
        /* simulate for one tine step, to see the effects after
         * adding source
        */
        Fluid.simulationStep();
        /* To see the fluid flow, we need to plot the density (dye)
         * value at every grid cell (except the border cells). move
         * the attribute array to color array
        */
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                /* cellAlpha has to be in the range 0.0 to 1.0
                */
                cellAlpha = Fluid.dPrev[(i + (j * N))];
                genCellColor(i + 1, j + 1, cellR, cellG, cellB, cellAlpha);
            }
        }

        /* move color array to GPU
        */
        moveDataToGPU(COLOR);
        /* Do we want the data rendered as a collection of points, 
         * a collection of triangles or perhaps just one long line? 
         * Those hints are called primitives and are given to OpenGL 
         * while calling any of the drawing commands. Some of these 
         * hints are GL_POINTS, GL_TRIANGLES and GL_LINE_STRIP.
         * 
         * The glDrawArrays function takes as its first argument 
         * the OpenGL primitive type we would like to draw. 
         * The second argument specifies the starting index of the 
         * vertex array we'd like to draw; we just leave this at 0. 
         * The last argument specifies how many vertices we want to 
         * draw
         * 
         * Instead of glDrawArrays, we use glDrawElement because of
         * EBO. The first argument specifies the mode we want to draw 
         * in, similar to glDrawArrays. The second argument is the 
         * count or number of elements we'd like to draw. The third 
         * argument is the type of the indices which is of type 
         * GL_UNSIGNED_INT. The last argument allows us to specify 
         * an offset in the EBO (or pass in an index array, but 
         * that is when you're not using element buffer objects), 
         * but we're just going to leave this at 0.
        */
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        /* The glfwSwapBuffers will swap the color buffer (a large 
         * 2D buffer that contains color values for each pixel in 
         * GLFW's window) that is used to render to during this render 
         * iteration and show it as output to the screen.
         * 
         * When an application draws in a single buffer the resulting 
         * image may display flickering issues. This is because the 
         * resulting output image is not drawn in an instant, but drawn 
         * pixel by pixel and usually from left to right and top to 
         * bottom. Because this image is not displayed at an instant 
         * to the user while still being rendered to, the result may 
         * contain artifacts. To circumvent these issues, windowing 
         * applications apply a double buffer for rendering. The front 
         * buffer contains the final output image that is shown at the 
         * screen, while all the rendering commands draw to the back 
         * buffer. As soon as all the rendering commands are finished 
         * we swap the back buffer to the front buffer so the image can 
         * be displayed without still being rendered to, removing all 
         * the aforementioned artifacts.
        */
        glfwSwapBuffers(window);
        /* The glfwPollEvents function checks if any events are 
         * triggered (like keyboard input or mouse movement events), 
         * updates the window state, and calls the corresponding 
         * functions (which we can register via callback methods). 
        */
        glfwPollEvents();
    }
    /* deallocate resources
    */
    openGLClose();
    return 0;
}
