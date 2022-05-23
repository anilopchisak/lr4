#ifndef SHADOW_MAP_TECHNIQUE_H
#define	SHADOW_MAP_TECHNIQUE_H

#include "technique.h"
#include "math_3d.h"

static const char* vertex_SM = R"(                                                          
#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
                                                                                    
uniform mat4 gWVP;                                                                  
                                                                                    
out vec2 TexCoordOut;                                                               
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position = gWVP * vec4(Position, 1.0);                                       
    TexCoordOut = TexCoord;                                                         
})";

static const char* fragment_SM = R"(                                                          
#version 330                                                                        
                                                                                    
in vec2 TexCoordOut;                                                                
uniform sampler2D gShadowMap;                                                       
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    float Depth = texture(gShadowMap, TexCoordOut).x;                               
    Depth = 1.0 - (1.0 - Depth) * 25.0;                                             
    FragColor = vec4(Depth);                                                        
})";

class ShadowMapTechnique : public Technique {

public:
    ShadowMapTechnique() { }

    bool Init()
    {
        if (!Technique::Init()) {
            return false;
        }

        if (!AddShader(GL_VERTEX_SHADER, vertex_SM)) {
            return false;
        }

        if (!AddShader(GL_FRAGMENT_SHADER, fragment_SM)) {
            return false;
        }

        if (!Finalize()) {
            return false;
        }

        m_WVPLocation = GetUniformLocation("gWVP");
        m_textureLocation = GetUniformLocation("gShadowMap");

        if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
            m_textureLocation == INVALID_UNIFORM_LOCATION) {
            return false;
        }

        return true;
    }

    void SetWVP(const Matrix4f& WVP)
    {
        glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
    }

    void SetTextureUnit(unsigned int TextureUnit)
    {
        glUniform1i(m_textureLocation, TextureUnit);
    }

private:

    GLuint m_WVPLocation;
    GLuint m_textureLocation;
};

#endif	/* SHADOW_MAP_TECHNIQUE_H */