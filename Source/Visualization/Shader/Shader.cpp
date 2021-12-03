#include "../../../Include/Visualization/Shader/Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderClass::ShaderClass(void){
    /* retrieve the vertex/fragment source code from 
     * filePath
    */
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    /* ensure ifstream objects can throw exceptions
    */
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try {
        /* open shader files
        */
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, fShaderStream;
        /* read file's buffer contents into streams
        */
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        /* close file handlers
        */
        vShaderFile.close();
        fShaderFile.close();

        /* convert stream into string
        */
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e){
        std::cout << "[ERROR] Shader file not successfully read" << std::endl;
    }

    /* Returns a pointer to an array that contains a 
     * null-terminated sequence of characters (i.e., a 
     * C-string) representing the current value of the 
     * string object.
    */
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();

    /* In order for OpenGL to use the shader it has to 
     * dynamically compile it at run-time from its 
     * source code.
    */
    unsigned int vertex, fragment;

    /* The first thing we need to do is create a shader 
     * object, again referenced by an ID. So we store 
     * the vertex shader as an unsigned int and create 
     * the shader with glCreateShader
     * 
     * We provide the type of shader we want to create 
     * as an argument to glCreateShader. Since we're 
     * creating a vertex shader we pass in GL_VERTEX_SHADER.
    */
    vertex = glCreateShader(GL_VERTEX_SHADER);
    /* Next we attach the shader source code to the shader 
     * object and compile the shader. The glShaderSource 
     * function takes the shader object to compile to as 
     * its first argument. The second argument specifies 
     * how many strings we're passing as source code, which 
     * is only one. The third parameter is the actual source 
     * code of the vertex shader and we can leave the 4th 
     * parameter to NULL.
    */
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    /* You probably want to check if compilation was successful 
     * after the call to glCompileShader and if not, what errors 
     * were found so you can fix those. This fn does just that
    */
    checkCompileErrors(vertex, "VERTEX");

    /* repeat the above steps for fragment shader. The process 
     * for compiling a fragment shader is similar to the vertex 
     * shader, although this time we use the GL_FRAGMENT_SHADER 
     * constant as the shader type
    */
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    /* Both the shaders are now compiled and the only thing 
     * left to do is link both shader objects into a shader 
     * program that we can use for rendering.
     * 
     * A shader program object is the final linked version 
     * of multiple shaders combined. To use the recently 
     * compiled shaders we have to link them to a shader program 
     * object and then activate this shader program when rendering 
     * objects. The activated shader program's shaders will be 
     * used when we issue render calls.When linking the shaders 
     * into a program it links the outputs of each shader to the 
     * inputs of the next shader. This is also where you'll get 
     * linking errors if your outputs and inputs do not match.
     * 
     * The glCreateProgram function creates a program and 
     * returns the ID reference to the newly created program 
     * object. 
    */
    ID = glCreateProgram();
    /* Now we need to attach the previously compiled shaders 
     * to the program object and then link them with 
     * glLinkProgram
    */
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    /* delete the shaders as they're linked into our 
     * program now and no longer necessary
    */
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void ShaderClass::use(){ 
    /* The result after linking is a program object that 
     * we can activate by calling glUseProgram with the 
     * newly created program object as its argument. Every 
     * shader and rendering call after glUseProgram will 
     * now use this program object (and thus the shaders)
    */
    glUseProgram(ID); 
}

/* we query for the location of the uniform using 
 * glGetUniformLocation. We supply the shader program and 
 * the name of the uniform (that we want to retrieve the 
 * location from) to the query function. 
 * 
 * If glGetUniformLocation returns -1, it could not find the 
 * location. Lastly we can set the uniform value using the 
 * glUniform_ function. 
*/
void ShaderClass::setBool(const std::string &name, bool value) const{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void ShaderClass::setInt(const std::string &name, int value) const{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void ShaderClass::setFloat(const std::string &name, float value) const{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void ShaderClass::setVec4(const std::string &name, float x, float y, float z, float w) const{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}

void ShaderClass::checkCompileErrors(unsigned int shader, std::string type){
    int success;
    char infoLog[1024];
    if (type != "PROGRAM"){
        /* First we define an integer to indicate success 
         * and a storage container for the error messages 
         * (if any). Then we check if compilation was 
         * successful with glGetShaderiv. If compilation 
         * failed, we should retrieve the error message with 
         * glGetShaderInfoLog and print the error message.
        */
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "[ERROR] Shader compilation error of type: "<<type<<"\n"<<infoLog<<std::endl;
        }
    }
    else{
        /* Just like shader compilation we can also check if 
         * linking a shader program failed and retrieve the 
         * corresponding log. However, instead of using 
         * glGetShaderiv and glGetShaderInfoLog we now use 
         * glGetProgramiv andglGetProgramInfoLog
        */
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "[ERROR] Program linking error of type: "<< type<<"\n"<<infoLog<<std::endl;
        }
    }
}