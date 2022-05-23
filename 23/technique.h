#ifndef TEXHNIQUE_H
#define TEXHNIQUE_H

#include <GL/glew.h>
#include <list>
#include <stdio.h>
#include <string.h>

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

class Technique
{
private:

    GLuint ShaderProgram;
    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList shaderObjList;

public:

    Technique()
    {
        ShaderProgram = 0;
    }

    ~Technique()
    {
        for (ShaderObjList::iterator it = shaderObjList.begin(); it != shaderObjList.end(); it++)
            glDeleteShader(*it);

        if (ShaderProgram != 0)
        {
            glDeleteProgram(ShaderProgram);
            ShaderProgram = 0;
        }
    }

    bool Init()
    {
        ShaderProgram = glCreateProgram();

        if (ShaderProgram == 0)
        {
            std::cerr << "Error creating shader program\n";
            return false;
        }

        return true;
    }

    void Enable()
    {
        glUseProgram(ShaderProgram);
    }

protected:

    bool AddShader(GLenum ShaderType, const char* pShaderText)
    {
        GLuint ShaderObj = glCreateShader(ShaderType);

        if (ShaderObj == 0)
        {
            fprintf(stderr, "Error creating shader type %d\n", ShaderType);
            return false;
        }

        shaderObjList.push_back(ShaderObj);

        const GLchar* p[1];
        p[0] = pShaderText;
        GLint Lengths[1];
        Lengths[0] = strlen(pShaderText);
        glShaderSource(ShaderObj, 1, p, Lengths);

        glCompileShader(ShaderObj);

        GLint success;
        glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            GLchar InfoLog[1024];
            glGetShaderInfoLog(ShaderObj, 1024, nullptr, InfoLog);
            fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
            return false;
        }

        glAttachShader(ShaderProgram, ShaderObj);

        return true;
    }

    bool Finalize()
    {
        GLint Success = 0;
        GLchar ErrorLog[1024] = { 0 };

        glLinkProgram(ShaderProgram);

        glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
        if (Success == 0)
        {
            glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), nullptr, ErrorLog);
            std::cerr << "Error linking shader program " << ErrorLog << "\n";
            return false;
        }

        glValidateProgram(ShaderProgram);
        glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
        if (Success == 0)
        {
            glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), nullptr, ErrorLog);
            std::cerr << "Invalid shader program " << ErrorLog << "\n";
            return false;
        }

        // Óäàëÿåì ïðîìåæóòî÷íûå îáúåêòû øåéäåðîâ, êîòîðûå áûëè äîáàâëåíû â ïðîãðàììó
        for (ShaderObjList::iterator it = shaderObjList.begin(); it != shaderObjList.end(); it++)
            glDeleteShader(*it);

        shaderObjList.clear();

        return true;
    }

    GLint GetUniformLocation(const char* pUniformName)
    {
        GLint Location = glGetUniformLocation(ShaderProgram, pUniformName);

        if ((unsigned int)Location == 0xFFFFFFFF)
            std::cerr << "Warning! Unable to get the location of uniform '%s'\n" << pUniformName;

        return Location;
    }
};

#endif /* TEXHNIQUE_H */