#ifndef CONTROL_UTILS_H
#define CONTROL_UTILS_H
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
#include <vector>

/* externs, since these are used in main
*/
extern float borderR, borderG, borderB, borderAlpha;
extern float cellR, cellG, cellB, cellAlpha;
extern unsigned int VAO;
extern int cellX, cellY;
extern std::vector<unsigned int> indices;
/* enum to decide the type of data to be processed
*/
typedef enum{
    VERTEX, 
    COLOR
}dataType;

/* fn declarations since these are used outside
 * in main
*/
GLFWwindow* openGLBringUp(void);
void openGLClose(void);
void moveDataToGPU(dataType dtType);
void setVertexAttribute(dataType dtType);
void processInput(GLFWwindow* window);

void genCellVerticesWrapper(int i, int j);
void genCellColor(int i, int j, float r, float g, float b, float alpha);
float getRandomAmount(float start, float end);
int getIdx(int i, int j);
#endif /* CONTROL_UTILS_H
*/