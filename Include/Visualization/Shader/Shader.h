#ifndef VISUALIZATION_SHADER_H
#define VISUALIZATION_SHADER_H

#include "../../Include/Visualization/glad/glad.h"
#include <string>

/* THE GRAPHICS PIPELINE
 * The first part of the pipeline is the VERTEX SHADER 
 * that takes as input a single vertex. The main purpose 
 * of the vertex shader is to transform 3D coordinates 
 * into different 3D coordinates and the vertex shader 
 * allows us to do some basic processing on the vertex 
 * attributes.
 * 
 * The PRIMITIVE ASSEMBLY stage takes as input all the 
 * vertices (or vertex if GL_POINTS is chosen) from the 
 * vertex shader that form a primitive and assembles all 
 * the point(s) in the primitive shape given
 * 
 * The output of the primitive assembly stage is passed 
 * to the GEOMETRY SHADER. The geometry shader takes as 
 * input a collection of vertices that form a primitive 
 * and has the ability to generate other shapes by emitting 
 * new vertices to form new (or other) primitive(s). 
 * 
 * The output of the geometry shader is then passed on to 
 * the RASTERIZATION stage where it maps the resulting 
 * primitive(s) to the corresponding pixels on the final 
 * screen, resulting in fragments for the fragment shader 
 * to use. Before the fragment shaders run, clipping is 
 * performed. Clipping discards all fragments that are 
 * outside your view, increasing performance. A fragment 
 * in OpenGL is all the data required for OpenGL to render 
 * a single pixel.
 * 
 * The main purpose of the FRAGMENT SHADER is to calculate 
 * the final color of a pixel and this is usually the stage 
 * where all the advanced OpenGL effects occur. Usually the 
 * fragment shader contains data about the 3D scene that it 
 * can use to calculate the final pixel color (like lights, 
 * shadows, color of the light and so on).
 * 
 * After all the corresponding color values have been 
 * determined, the final object will then pass through one 
 * more stage that we call the ALPHA TEST AND BLENDING 
 * stage. This stage checks the corresponding depth (and 
 * stencil) value of the fragment and uses those to check 
 * if the resulting fragment is in front or behind other 
 * objects and should be discarded accordingly. The stage 
 * also checks for alpha values (alpha values define the 
 * opacity of an object) and blends the objects accordingly. 
 * So even if a pixel output color is calculated in the fragment 
 * shader, the final pixel color could still be something 
 * entirely different.
 * 
 * In modern OpenGL we are required to define at least 
 * a vertex and fragment shader of our own (there are 
 * no default vertex/fragment shaders on the GPU).
*/
class ShaderClass{
    private:
        /* shader file paths
        */
        const char* vertexPath = 
        "/Users/vijoys/Desktop/Projects/FluidSim/Source/Visualization/Shader/ShaderVert.sdr";
        const char* fragmentPath = 
        "/Users/vijoys/Desktop/Projects/FluidSim/Source/Visualization/Shader/ShaderFrag.sdr";
        /* Check compilation errors after compiling shaders
         * and linking errors after linking a shader program
        */
        void checkCompileErrors(GLuint shader, std::string type);
    public:
        /* id for the shader program object
        */
        unsigned int ID;
        /* Constructor that takes in the file paths for vertex
         * and fragment shaders source code
        */
        ShaderClass();
        /* Activate a shader program object referenced by ID
        */
        void use(void); 
        /* Utility uniform functions
         * Because OpenGL is in its core a C library it does not 
         * have native support for function overloading, so 
         * wherever a function can be called with different 
         * types OpenGL defines new functions for each type 
         * required; glUniform is a perfect example of this. 
         * The function requires a specific postfix for the 
         * type of the uniform you want to set. A few of the 
         * possible postfixes are:
         * f: the function expects a float as its value.
         * i: the function expects an int as its value.
         * ui: the function expects an unsigned int as its value.
         * 3f: the function expects 3 floats as its value.
         * fv: the function expects a float vector/array as its value.
         * Whenever you want to configure an option of OpenGL 
         * simply pick the overloaded function that corresponds 
         * with your type.
        */
        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec4(const std::string &name, float x, float y, float z, float w) const;
};
#endif /* VISUALIZATION_SHADER_H
*/