#include "../Include/Visualization/Shader/Shader.h"
#include "../Include/Simulation/Fluid.h"
/* Be sure to include GLAD before GLFW. The include file 
 * for GLAD includes the required OpenGL headers behind 
 * the scenes (like GL/gl.h) so be sure to include GLAD 
 * before other header files that require OpenGL (like GLFW)
*/

/* Because OpenGL is only really a standard/specification 
 * it is up to the driver manufacturer to implement the 
 * specification to a driver that the specific graphics 
 * card supports. Since there are many different versions 
 * of OpenGL drivers, the location of most of its functions 
 * is not known at compile-time and needs to be queried 
 * at run-time. It is then the task of the developer to 
 * retrieve the location of the functions he/she needs 
 * and store them in function pointers for later use.
 * 
 * GLAD is an open source library that manages all that 
 * cumbersome work we talked about.
*/
#include <glad/glad.h>
/* OpenGL is by itself a large state machine: 
 * a collection of variables that define how OpenGL should 
 * currently operate. The state of OpenGL is commonly 
 * referred to as the OpenGL CONTEXT. 
 * 
 * When using OpenGL, we often change its state by setting 
 * some options, manipulating some buffers and then render 
 * using the current context.
 * 
 * The first thing we need to do before we start creating 
 * stunning graphics is to create an OpenGL context and an 
 * application window to draw in. 
 * 
 * However, those operations are specific per operating 
 * system and OpenGL purposefully tries to abstract itself 
 * from these operations. This means we have to create a 
 * window, define a context, and handle user input all by 
 * ourselves.
 * 
 * GLFW gives us the bare necessities required for rendering 
 * goodies to the screen. It allows us to create an OpenGL 
 * context, define window parameters, and handle user input, 
 * which is plenty enough for our purposes.
*/
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

/* create fluid object
*/
FluidClass Fluid(N, dDiff, vDiff, dt);
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const char* windowTitle = "FLUID SIM";

int main()
{
    /* This fn initializes the GLFW library. This has to 
     * be done before most GLFW fns can be used
    */
    glfwInit();
    /* Configure GLFW, the first argument tells us what 
     * option we want to configure, where we can select the 
     * option from a large enum of possible options prefixed 
     * with GLFW_. 
     * 
     * The second argument is an integer that sets the value 
     * of our option.Here we'd like to tell GLFW that 3.3 is 
     * the OpenGL version we want to use. This way GLFW can 
     * make the proper arrangements when creating the OpenGL 
     * context. 
     * 
     * This ensures that when a user does not have the proper 
     * OpenGL version GLFW fails to run. We set the major and 
     * minor version both to 3. We also tell GLFW we want to 
     * explicitly use the core-profile. Telling GLFW we want 
     * to use the core-profile means we'll get access to a 
     * smaller subset of OpenGL features without backwards-
     * compatible features we no longer need.
    */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Note that on Mac OS X you need to add 
    * glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    * to your initialization code for it to work.
    */
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    /* Next we're required to create a window object. 
     * This window object holds all the windowing data and 
     * is required by most of GLFW's other functions.
     * 
     * This fn requires the window width and height as its 
     * first two arguments respectively. The third argument 
     * allows us to create a name for the window, we can 
     * ignore the remaining 2 arguments
     * 
     * The function returns a GLFWwindow object that we'll 
     * later need for other GLFW operations. 
    */
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle, NULL, NULL);
    if (window == NULL){
        std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    /* Next we tell GLFW to make the context of our window 
     * the main context on the current thread.
    */
    glfwMakeContextCurrent(window);
    /* We have to tell GLFW we want to call this function 
    * on every window resize by registering it
    */
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* GLAD manages function pointers for OpenGL so we want 
     * to initialize GLAD before we call any OpenGL function.
     * We pass GLAD the function to load the address of the 
     * OpenGL function pointers which is OS-specific. GLFW 
     * gives us glfwGetProcAddress that defines the correct 
     * function based on which OS we're compiling for.
    */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "[ERROR] Failed to initialize GLAD" << std::endl;
        return -1;
    }

    /* Build and compile the shader program
    */
    ShaderClass Shader("/Users/vijoys/Desktop/Projects/FluidSim/Source/Visualization/Shader/ShaderVert.sdr", 
                    "/Users/vijoys/Desktop/Projects/FluidSim/Source/Visualization/Shader/ShaderFrag.sdr");

    /* As input to the graphics pipeline we pass in a list
     * of 3D coordinates that should form the desired shape
     * in an array here called VERTEX DATA; this vertex data 
     * is a collection of vertices. 
     * 
     * A vertex is a collection of data per 3D coordinate. 
     * This vertex's data is represented using VERTEX 
     * ATTRIBUTES that can contain any data we'd like, but 
     * for simplicity's sake let's assume that each vertex 
     * consists of just a 3D position and some color value.
     * 
     * NOTE: OpenGL only processes 3D coordinates when they're 
     * in a specific range between -1.0 and 1.0 on all 3 axes 
     * (x, y and z). All coordinates within this so called 
     * normalized device coordinates range will end up visible 
     * on your screen (and all coordinates outside this region 
     * won't).
    */
    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };

    /* With the vertex data defined we'd like to send it as
     * input to the first process of the graphics pipeline: 
     * the vertex shader.
     * 
     * This is done by creating memory on the GPU where we 
     * store the vertex data, configure how OpenGL should 
     * interpret the memory and specify how to send the data 
     * to the graphics card. The vertex shader then processes 
     * as much vertices as we tell it to from its memory.
     * 
     * We manage this memory via so called vertex buffer 
     * objects (VBO) that can store a large number of vertices 
     * in the GPU's memory. The advantage of using those 
     * buffer objects is that we can send large batches of 
     * data all at once to the graphics card, and keep it 
     * there if there's enough memory left, without having 
     * to send data one vertex at a time. Sending data to 
     * the graphics card from the CPU is relatively slow, 
     * so wherever we can we try to send as much data as 
     * possible at once. Once the data is in the graphics 
     * card's memory the vertex shader has almost instant 
     * access to the vertices making it extremely fast
    */
    unsigned int VBO, VAO;
    /* A vertex array object (also known as VAO) can be 
     * bound just like a vertex buffer object, and any 
     * subsequent vertex attribute calls from that point 
     * on will be stored inside the VAO. This has the 
     * advantage that when configuring vertex attribute 
     * pointers you only have to make those calls once 
     * and whenever we want to draw the object, we can 
     * just bind the corresponding VAO. This makes switching 
     * between different vertex data and attribute 
     * configurations as easy as binding a different VAO. 
     * All the state we just set is stored inside the VAO.
     * 
     * NOTE: Core OpenGL requires that we use a VAO so it 
     * knows what to do with our vertex inputs. If we fail 
     * to bind a VAO, OpenGL will most likely refuse to 
     * draw anything.
     * 
     * To use a VAO all you have to do is bind the VAO 
     * using glBindVertexArray. From that point on we should 
     * bind/configure the corresponding VBO(s) and attribute 
     * pointer(s) and then unbind the VAO for later use. 
     * As soon as we want to draw an object, we simply bind 
     * the VAO with the preferred settings before drawing 
     * the object and that is it.
    */
    glGenVertexArrays(1, &VAO);
    /* A vertex buffer object is an OpenGL object. Just like 
     * any object in OpenGL, this buffer has a unique ID 
     * corresponding to thatbuffer, so we can generate one 
     * with a buffer ID using the glGenBuffers function. 
    */
    glGenBuffers(1, &VBO);
    /* Bind the Vertex Array Object first, then bind and 
     * set vertex buffer(s), and then configure vertex 
     * attributes(s).
    */
    glBindVertexArray(VAO);
    /* OpenGL has many types of buffer objects and the 
     * buffer type of a vertex buffer object is 
     * GL_ARRAY_BUFFER. OpenGL allows us to bind to several 
     * buffers at once as long as they have a different 
     * buffer type. We can bind the newly created buffer to 
     * the GL_ARRAY_BUFFER target with the glBindBuffer 
     * function
     * 
     * From that point on any buffer calls we make (on the 
     * GL_ARRAY_BUFFER target) will be used to configure 
     * the currently bound buffer, which is VBO.
    */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /* This fn copies the previously defined vertex data 
     * into the buffer's memory.
     * 
     * glBufferData is a function specifically targeted 
     * to copy user-defined data into the currently bound 
     * buffer. Its first argument is the type of the buffer 
     * we want to copy data into: the vertex buffer object 
     * currently bound to the GL_ARRAY_BUFFER target. The 
     * second argument specifies the size of the data (in 
     * bytes) we want to pass to the buffer; a simple sizeof 
     * of the vertex data suffices. The third parameter is 
     * the actual data we want to send.
     * 
     * The fourth parameter specifies how we want the graphics 
     * card to manage the given data. This can take 3 forms:
     * 
     * GL_STREAM_DRAW: the data is set only once and used 
     * by the GPU at most a few times.
     * GL_STATIC_DRAW: the data is set only once and used 
     * many times.
     * GL_DYNAMIC_DRAW: the data is changed a lot and used 
     * many times.
     * 
     * NOTE: If, for instance, one would have a buffer with 
     * data that is likely to change frequently, a usage type 
     * of GL_DYNAMIC_DRAW ensures the graphics card will 
     * place the data in memory that allows for faster writes.
     * 
     * As of now we stored the vertex data within memory on 
     * the graphics card as managed by a vertex buffer object 
     * named VBO. Next we want to create a vertex and fragment 
     * shader that actually processes this data.
    */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Right now we sent the input vertex data to the GPU 
     * and instructed the GPU how it should process the 
     * vertex data within a vertex and fragment shader. We're 
     * almost there, but not quite yet. OpenGL does not yet 
     * know how it should interpret the vertex data in memory 
     * and how it should connect the vertex data to the vertex 
     * shader's attributes.
    */
    
    /* The vertex shader allows us to specify any input we 
     * want in the form of vertex attributes and while 
     * this allows for great flexibility, it does mean we 
     * have to manually specify what part of our input data 
     * goes to which vertex attribute in the vertex shader. 
     * This means we have to specify how OpenGL should 
     * interpret the vertex data before rendering.
     * 
     * The first parameter specifies which vertex attribute 
     * we want to configure. For example, if we specified the 
     * location of the position vertex attribute in the 
     * vertex shader with layout (location = 0), it sets 
     * the location of the vertex attribute to 0 and if
     * we want to pass data to this vertex attribute, we 
     * pass in 0.
     * 
     * The next argument specifies the size of the vertex 
     * attribute. (example: if vec3, 3 is the size)
     * 
     * The third argument specifies the type of the data
     * 
     * The next argument specifies if we want the data to 
     * be normalized. If we're inputting integer data types 
     * (int, byte) and we've set this to GL_TRUE, the integer 
     * data is normalized to 0 (or -1 for signed data) and 
     * 1 when converted to float.
     * 
     * The fifth argument is known as the stride and tells 
     * us the space between consecutive vertex attributes. 
     * 
     * The last parameter is of type void* and thus requires 
     * that weird cast. This is the offset of where the 
     * position data begins in the buffer. For example,
     * if the position data is at the start of the data 
     * array this value is just 0.
     * 
     * NOTE: Each vertex attribute takes its data from memory 
     * managed by a VBO and which VBO it takes its data from 
     * (you can have multiple VBOs) is determined by the VBO 
     * currently bound to GL_ARRAY_BUFFER when calling 
     * glVertexAttribPointer
    */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    /* Now that we specified how OpenGL should interpret the 
     * vertex data we should also enable the vertex attribute 
     * with glEnableVertexAttribArray giving the vertex 
     * attribute location as its argument; vertex attributes 
     * are disabled by default.
    */
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    /* NOTE: You can unbind the VAO afterwards so other VAO 
     * calls won't accidentally modify this VAO, but this 
     * rarely happens. Modifying other VAOs requires a call 
     * to glBindVertexArray anyways so we generally don't 
     * unbind VAOs (nor VBOs) when it's not directly necessary.
     * glBindVertexArray(0);
    */


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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Use the shader object that we linked using our shader 
         * files
        */
        Shader.use();
        glBindVertexArray(VAO);
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
        */
        glDrawArrays(GL_TRIANGLES, 0, 3);

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

    /* de-allocate all resources once they've outlived 
     * their purpose
    */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    /* As soon as we exit the render loop we would like 
     * to properly clean/delete all of GLFW's resources 
     * that were allocated. We can do this via the glfwTerminate 
     * function that we call at the end of the main function.
    */
    glfwTerminate();
    return 0;
}

/* query GLFW whether relevant keys are pressed/released 
 * this frame and react accordingly. Here we check whether 
 * the user has pressed the escape key (if it's not pressed, 
 * glfwGetKey returns GLFW_RELEASE). If the user did press 
 * the escape key, we close GLFW by setting its WindowShouldClose 
 * property to true using glfwSetwindowShouldClose. The next 
 * condition check of the main while loop will then fail and 
 * the application closes.
*/
void processInput(GLFWwindow *window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

/* whenever the window size changed (by OS or user resize) 
 * this callback function executes. The framebuffer size 
 * function takes a GLFWwindow as its first argument and 
 * two integers indicating the new window dimensions. 
 * Whenever the window changes in size, GLFW calls this 
 * function and fills in the proper arguments for you to 
 * process.
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    /* We have to tell OpenGL the size of the rendering 
     * window so OpenGL knows how we want to display the 
     * data and coordinates with respect to the window. 
     * We can set those dimensions via the glViewport 
     * function
     * 
     * The first two parameters of glViewport set the 
     * location of the lower left corner of the window. 
     * The third and fourth parameter set the width and 
     * height of the rendering window in pixels, which we 
     * set equal to GLFW's window size.
     * 
     * We could actually set the viewport dimensions at 
     * values smaller than GLFW's dimensions; then all 
     * the OpenGL rendering would be displayed in a smaller 
     * window and we could for example display other 
     * elements outside the OpenGL viewport.
     * 
     * Note that processed coordinates in OpenGL are between
     * -1 and 1 so we effectively map from the range (-1 to 
     * 1) to (0, 800) and (0, 600).
    */
    glViewport(0, 0, width, height);
}