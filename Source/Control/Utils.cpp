#include "../../Include/Control/Utils.h"
#include "../../Include/Control/Constants.h"
#include <random>
#include <stdlib.h> /* for malloc, calloc, free
*/
#include <iostream>

/* OpenGL only processes 3D coordinates when they're 
 * in a specific range between -1.0 and 1.0 on all 3 axes 
 * (x, y and z). All coordinates within this so called 
 * normalized device coordinates range will end up visible 
 * on your screen (and all coordinates outside this region 
 * won't).
*/
const float yMin = -1.0, yMax = 1.0;
const float xMin = -1.0, xMax = 1.0; 
/* grid cell dimension, depends on number of grids we
 * are simulating with the fluid
*/
const float cellSize = (xMax - xMin)/(N + 2);
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
*/
std::vector<float> vertices;
/* These indices tells the order to draw the quad using 
 * 4 vertices using EBO
*/
std::vector<unsigned int> indices;
/* RGBA format, define the color for all the border cells.
 * We are not using vector here since we need to index
 * into a specific cell to set the colors. It is easier
 * to do this with arrays
 * 
 * Total #of elements in color array = 
 * 4 (RGBA) * 4(vertices per cell) * 
 * (N + 2) * (N + 2) (cells)
*/
const int sz = 16 * (N + 2) * (N + 2);
float *color = (float*)malloc(sizeof(float) * sz);
/* cell colors
*/
float borderR = 1.0, borderG = 1.0, borderB = 0.0, borderAlpha = 1.0;
float cellR = 1.0, cellG = 1.0, cellB = 1.0, cellAlpha = 0.0;
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
 *  
 * We will also be using EBO, An EBO is a buffer, just 
 * like a vertex buffer object, that stores indices 
 * that OpenGL uses to decide what vertices to draw. 
 * This also called indexed drawing. This can help us
 * reduce the number of vertices to define when drawing
 * quads using triangles
 * 
 * We will be using two VBOs, one for vertices and the
 * other for color
*/
unsigned int VBO, VBOColor, VAO, EBO;
/* this is used in generating indices and also to insert
 * color to vertex/cell identified by its eboIdx
*/
int eboIdx = 0;
/* grid cell position at mouse click, default set to the
 * middle cell
*/
int cellX = N/2, cellY = N/2;

/* function declarations
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void genBufferObjects(void);
void genCellVertices(float i, float j);
int getEBOIdx(int i, int j);

GLFWwindow* openGLBringUp(void){
    /* Total screen space, we do N+2 because we will
     * be drawing border cells as well
    */
    const unsigned int screenWidth = (N + 2) * scale;
    const unsigned int screenHeight = (N + 2) * scale;
    /* main render window title
    */
    const char* windowTitle = "FLUID SIM";
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, NULL, NULL);
    if (window == NULL){
        std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    /* Next we tell GLFW to make the context of our window 
     * the main context on the current thread.
    */
    glfwMakeContextCurrent(window);
    /* We have to tell GLFW we want to call this function 
     * on every mouse click
    */
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    /* GLAD manages function pointers for OpenGL so we want 
     * to initialize GLAD before we call any OpenGL function.
     * We pass GLAD the function to load the address of the 
     * OpenGL function pointers which is OS-specific. GLFW 
     * gives us glfwGetProcAddress that defines the correct 
     * function based on which OS we're compiling for.
    */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "[ERROR] Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    /* gnereate buffers
    */
    genBufferObjects();
    return window;
}

void genBufferObjects(void){
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
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBOColor);
}

/* bind the right buffer and move data to GPU, unbind after
 * data transfer
*/
void moveDataToGPU(dataType dtType){
    if(dtType == VERTEX){
        /* Bind the Vertex Array Object first, then bind and 
         * set vertex buffer(s)
         * 
         * Bind VAO-----|
         *              |----Bind VBO
         *                      |
         *                      |-------- Transfer Vertex data
         *                      |
         *              |----Bind EBO
         *                      |
         *                      |-------- Transfer Index data
         *                      |
         * Unbind VAO-----------|
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
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    static_cast<void*>(vertices.data()), GL_STATIC_DRAW);
        /* Copy the index array into element buffer
        */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), 
                    static_cast<void*>(indices.data()), GL_STATIC_DRAW);
        /* NOTE: do NOT unbind the BO while a VAO is active as the 
         * bound element buffer object IS stored in the VAO
        */
        /* NOTE: You can unbind the VAO afterwards so other VAO 
         * calls won't accidentally modify this VAO, but this 
         * rarely happens. Modifying other VAOs requires a call 
         * to glBindVertexArray anyways so we generally don't 
         * unbind VAOs (nor VBOs) when it's not directly necessary.
        */ 
        glBindVertexArray(0);
    }

    else if(dtType == COLOR){
        /* Bind the Vertex Array Object first, then bind and 
         * set vertex buffer(s)
         * 
         * Bind VAO-----|
         *              |----Bind VBOColor
         *                      |
         *                      |-------- Transfer Color data
         *                      |
         * Unbind VAO-----------|
        */
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOColor);
        glBufferData(GL_ARRAY_BUFFER, sz * sizeof(float), color, GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
    }
}

void setVertexAttribute(dataType dtType){
    if(dtType == VERTEX){
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        /* The first parameter specifies which vertex attribute 
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (sizeof(float)), (void*)0);
        /* Now that we specified how OpenGL should interpret the 
        * vertex data we should also enable the vertex attribute 
        * with glEnableVertexAttribArray giving the vertex 
        * attribute location as its argument; vertex attributes 
        * are disabled by default.
        */
        glEnableVertexAttribArray(0);
        glBindVertexArray(0); 
    }
    else if(dtType == COLOR){
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOColor);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * (sizeof(float)), (void*)0);
        glEnableVertexAttribArray(1);                  
        glBindVertexArray(0); 
    }
}

void openGLClose(void){
    /* de-allocate all resources once they've outlived 
     * their purpose
    */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBOColor);
    /* deallocate heap memory
    */
    free(color);
    /* As soon as we exit the render loop we would like 
     * to properly clean/delete all of GLFW's resources 
     * that were allocated. We can do this via the glfwTerminate 
     * function that we call at the end of the main function.
    */
    glfwTerminate();
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
void processInput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

/* call back function upon mouse click. if this is the
 * screen space:
 * ------------------------------------- X axis
 * |(0,0)                           ((N+2)*scale,0)              
 * |                                    |
 * |                                    |
 * |                                    |
 * |(0, (N+2)*scale)                ((N+2)*scale, (N+2)*scale)
 * Y axis
 * 
 * What we need is to convert xPos and yPos to grid cell
 * coordinates, and set them to cellX, cellY
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xPos, yPos;
        /* getting cursor position
        */
        glfwGetCursorPos(window, &xPos, &xPos);
        /* First we remove the scale factor from the position
         * returned
        */
        xPos = xPos/scale;
        yPos = yPos/scale;
#if 0
        std::cout << "Cursor Position at " << xPos << " : " << xPos << std::endl;
#endif
        /* compute grid cell position
        */
        cellX = N/2;
        cellY = N/2;
    }    
}

/* (i,j) will be the top left coordinates of a grid cell
 * pass in (i,j) to get vertices based on all variable 
 * parameters of the render screen. 
 * 
 * we need our grid cells to align as below
 * ----------------(+1)--------------
 * |        |       |       |       |
 * ----------------------------------
 * |        |       |       |       |
 * (-1)-------------0--------------(1)
 * |        |       |       |       |
 * ----------------------------------
 * |        |       |       |       |
 * ----------------(-1)--------------      
 * The function generates the 4 vertices required to draw
 * 2 triangles forming a quad. This also generates the 
 * indices vector that specifies the order in which these
 * vertices are to be drawn.
*/
void genCellVertices(float i, float j){
    /* set top left first
    */
    vertices.push_back(i);
    vertices.push_back(j);
    vertices.push_back(0.0);
    /* bottom left
    */
    vertices.push_back(i);
    vertices.push_back(j - cellSize);
    vertices.push_back(0.0);
    /* bottom right
    */
    vertices.push_back(i + cellSize);
    vertices.push_back(j - cellSize);
    vertices.push_back(0.0);
    /* top right
    */
    vertices.push_back(i + cellSize);
    vertices.push_back(j);
    vertices.push_back(0.0);

    /* create indices to specify the order in
     * which the above vertives are plotted
    */
    indices.push_back(eboIdx);
    indices.push_back(eboIdx + 1);
    indices.push_back(eboIdx + 3);

    indices.push_back(eboIdx + 1);
    indices.push_back(eboIdx + 2);
    indices.push_back(eboIdx + 3);
    eboIdx += 4;
}

/* This function accepts (i,j) in fluid grid co-ordinate
 * space. We need to transform to screen space before
 * invoking the sub function
*/
void genCellVerticesWrapper(int i, int j){
    /* convert 2d fluid coordinates to 1D, this will
     * range from 0 to (N+2)*(N+2)-1
     * For example: let N + 2 = 4
     * So idx ranges from 0 to 15
    */
    int idx = i + (N + 2) * j;
    /* first lets place idx 0 to (N+2)-1 cellSize apart
     * then (N+2) to 2(N+2)-1 cellsize apart, repeat this
     * till ((N+2)-1)(N+2) to ((N+2)(N+2))-1. Lets call
     * this row based placement
     *                          -1
     *                          |----------------------------------------
     *                          | 1.5,0   | 1.5,0.5 | 1.5,1.0 | 1.5,1.5 |
     *                          |----------------------------------------
     *                          | 1.0,0   | 1.0,0.5 | 1.0,1.0 | 1.0,1.5 |
     *                          |----------------------------------------
     *                          | 0.5,0   | 0.5,0.5 | 0.5,1.0 | 0.5,1.5 |
     *                          |----------------------------------------
     *                          | 0,0     | 0,0.5   | 0,1.0   | 0,1.5   | 
     * -1 ----------------------0---------------------------------------- +1
     *                          |
     *                          |
     *                          -1
     * Now that all points are in the screen space, they
     * are still in the first quadrant, we need to translate
     * them to lower left so that (0,0) is at the lower left
     * 
     * Finally, we need to shift by cellSize upwards to get
     * the top left vertex
    */
    float rowIdx = (idx / (N + 2)) * cellSize;
    float colIdx = (idx % (N + 2)) * cellSize;

    float x = rowIdx - ((N + 2)/2) * cellSize;
    float y = colIdx - ((N + 2)/2) * cellSize;
    
    y = y + cellSize;
    genCellVertices(x, y);
}

/* get ending eboIdx from cell location, makes it easier 
 * to set color of a cell based on its location
 * ---------------- .....
 * | 0,0  | 1,0   | .....
 * ---------------- .....
 *        ^       ^
 * eboIdx = 4     eboIdx = 8
*/
int getEBOIdx(int i, int j){
    return 4 + ((i + (N + 2) * j) * 4);
}

/* generate color value for a cell by adding color to
 * all 4 vertices. This fn will be called after generating 
 * all 4 vertices of a cell
 *
 *         v0       v1       v2       v3
 *         x,y,z    x,y,z    x,y,z    x,y,z
 *         r,g,b,a  r,g,b,a  r,g,b,a  r,g,b,a
 *                                           ^
 *                                           |
 *                                           eboIdx = 4
*/
void genCellColor(int i, int j, float r, float g, float b, float alpha){
    int n = getEBOIdx(i, j) * 4;
    int cellColorBatchSize = 16; /* 4 vertices, 4 color values
    */
    int k = n - cellColorBatchSize;
    /* set color for all 4 vertices  from top left,
     * bottom left, bottom right and top right
    */
    for(int i = 0; i < 4; i++){
        color[k++] = r;
        color[k++] = g;
        color[k++] = b;
        color[k++] = alpha;       
    }
}

/* given a start and an end range, generate a random number.
 * A use case for this function is to add sources upon mouse
 * click
*/
float getRandomAmount(float start, float end){
    /* At first, the std::random_device object should be 
     * initialized. It produces non-deterministic random bits 
     * for random engine seeding, which is crucial to avoid 
     * producing the same number sequences. Here we use std::
     * default_random_engine to generate pseudo-random values, 
     * but you can declare specific algorithm engine. Next, we
     *  initialize a uniform distribution and pass min/max values
     *  as optional arguments.
    */
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<> distr(start, end);
    return distr(eng);
}

/* get grid position given the index
 * positions (i, j)
 * Usage: if i = 9, j = 9 in a 10x10 grid,
 * index will be 9 + (9 * 10) = 99
*/
int getIdx(int i, int j){
    return i + (j * N);
}