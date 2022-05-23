#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "technique.h"

static const char* pVSName = "VS";
static const char* pFSName = "FS";

const char* ShaderType2ShaderName(GLuint Type)
{
    switch (Type) {
        case GL_VERTEX_SHADER:
            return pVSName;
        case GL_FRAGMENT_SHADER:
            return pFSName;
        default:
            assert(0);
    }

    return NULL;
}
Technique::Technique()
{
    shaderProg = 0;
}


Technique::~Technique()
{
    // Delete the intermediate shader objects that have been added to the program
    // The list will only contain something if shaders were compiled but the object itself
    // was destroyed prior to linking.
    for (ShaderObjList::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++)
    {
        glDeleteShader(*it);
    }

    if (shaderProg != 0)
    {
        glDeleteProgram(shaderProg);
        shaderProg = 0;
    }
}


bool Technique::Init()
{
    shaderProg = glCreateProgram();

    if (shaderProg == 0) {
        fprintf(stderr, "Error creating shader program\n");
        return false;
    }

    return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool Technique::AddShader(GLenum ShaderType, const char* pShaderText)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        return false;
    }

    // Save the shader object - will be deleted in the destructor
    shaderObjList.push_back(ShaderObj);

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0]= strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling %s: '%s'\n", ShaderType2ShaderName(ShaderType), InfoLog);
        return false;
    }

    glAttachShader(shaderProg, ShaderObj);

    return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Technique::Finalize()
{
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(shaderProg);

    glGetProgramiv(shaderProg, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        return false;
	}

    glValidateProgram(shaderProg);
    glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }

    // Delete the intermediate shader objects that have been added to the program
    for (ShaderObjList::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++)
    {
        glDeleteShader(*it);
    }

    shaderObjList.clear();

    return true;
}


void Technique::Enable()
{
    glUseProgram(shaderProg);
}


GLint Technique::GetUniformLocation(const char* pUniformName)
{
    GLint Location = glGetUniformLocation(shaderProg, pUniformName);

    if (Location == (GLint)0xFFFFFFFF) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}