#ifndef LIGHTING_TECHNIQUE_H
#define	LIGHTING_TECHNIQUE_H

#include "technique.h"
#include "math_3d.h"
#include "util.h"

#include <glm\glm.hpp>
#include <limits.h>
#include <string.h>

static const unsigned int MAX_POINT_LIGHTS = 2;
static const unsigned int MAX_SPOT_LIGHTS = 2;

static const char* vertex_LT = R"(                                                          
#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
                                                                                    
uniform mat4 gWVP;                                                                  
uniform mat4 gLightWVP;                                                             
uniform mat4 gWorld;                                                                
                                                                                    
out vec4 LightSpacePos;                                                             
out vec2 TexCoord0;                                                                 
out vec3 Normal0;                                                                   
out vec3 WorldPos0;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position      = gWVP * vec4(Position, 1.0);                                  
    LightSpacePos = gLightWVP * vec4(Position, 1.0);                                 
    TexCoord0        = TexCoord;                                                    
    Normal0          = (gWorld * vec4(Normal, 0.0)).xyz;                            
    WorldPos0        = (gWorld * vec4(Position, 1.0)).xyz;                               
})";

static const char* fragment_LT = R"(                                                          
#version 330                                                                        
                                                                                    
const int MAX_POINT_LIGHTS = 2;                                                     
const int MAX_SPOT_LIGHTS = 2;                                                      
                                                                                    
in vec4 LightSpacePos;                                                              
in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    
in vec3 WorldPos0;                                                                  
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
struct BaseLight                                                                    
{                                                                                   
    vec3 Color;                                                                     
    float AmbientIntensity;                                                         
    float DiffuseIntensity;                                                         
};                                                                                  
                                                                                    
struct DirectionalLight                                                             
{                                                                                   
    BaseLight Base;                                                          
    vec3 Direction;                                                                 
};                                                                                  
                                                                                    
struct Attenuation                                                                  
{                                                                                   
    float Constant;                                                                 
    float Linear;                                                                   
    float Exp;                                                                      
};                                                                                  
                                                                                    
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                                                                  
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};                                                                                          
                                                                                            
struct SpotLight                                                                            
{                                                                                           
    PointLight Base;                                                                 
    vec3 Direction;                                                                         
    float Cutoff;                                                                           
};                                                                                          
                                                                                            
uniform int gNumPointLights;                                                                
uniform int gNumSpotLights;                                                                 
uniform DirectionalLight gDirectionalLight;                                                 
uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];                                             
uniform sampler2D gSampler;                                                                 
uniform sampler2D gShadowMap;                                                               
uniform vec3 gEyeWorldPos;                                                                  
uniform float gMatSpecularIntensity;                                                        
uniform float gSpecularPower;                                                               
                                                                                            
float CalcShadowFactor(vec4 LightSpacePos)                                                  
{                                                                                           
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;                                  
    vec2 UVCoords;                                                                          
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;                                                  
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;                                                  
    float z = 0.5 * ProjCoords.z + 0.5;                                                     
    float Depth = texture(gShadowMap, UVCoords).x;                                          
    if (Depth < z + 0.00001)                                                                 
        return 0.5;                                                                         
    else                                                                                    
        return 1.0;                                                                         
}                                                                                           
                                                                                            
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal,            
                       float ShadowFactor)                                                  
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;                   
    float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * DiffuseFactor;    
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                             
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                              
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
        if (SpecularFactor > 0) {                                                           
            SpecularColor = vec4(Light.Color, 1.0f) *                                       
                            gMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor));                  
}                                                                                           
                                                                                            
vec4 CalcDirectionalLight(vec3 Normal)                                                      
{                                                                                                
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, 1.0);  
}                                                                                                
                                                                                            
vec4 CalcPointLight(PointLight l, vec3 Normal, vec4 LightSpacePos)                   
{                                                                                           
    vec3 LightDirection = WorldPos0 - l.Position;                                           
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
    float ShadowFactor = CalcShadowFactor(LightSpacePos);                                   
                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal, ShadowFactor);           
    float Attenuation =  l.Atten.Constant +                                                 
                         l.Atten.Linear * Distance +                                        
                         l.Atten.Exp * Distance * Distance;                                 
                                                                                            
    return Color / Attenuation;                                                             
}                                                                                           
                                                                                            
vec4 CalcSpotLight(SpotLight l, vec3 Normal, vec4 LightSpacePos)                     
{                                                                                           
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);                             
    float SpotFactor = dot(LightToPixel, l.Direction);                                      
                                                                                            
    if (SpotFactor > l.Cutoff) {                                                            
        vec4 Color = CalcPointLight(l.Base, Normal, LightSpacePos);                         
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
    }                                                                                       
    else {                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}                                                                                           
                                                                                            
void main()                                                                                 
{                                                                                           
    vec3 Normal = normalize(Normal0);                                                       
    vec4 TotalLight = CalcDirectionalLight(Normal);                                         
                                                                                            
    for (int i = 0 ; i < gNumPointLights ; i++) {                                           
        TotalLight += CalcPointLight(gPointLights[i], Normal, LightSpacePos);               
    }                                                                                       
                                                                                            
    for (int i = 0 ; i < gNumSpotLights ; i++) {                                            
        TotalLight += CalcSpotLight(gSpotLights[i], Normal, LightSpacePos);                 
    }                                                                                       
                                                                                            
    vec4 SampledColor = texture2D(gSampler, TexCoord0.xy);                                  
    FragColor = SampledColor * TotalLight;                             
})";

struct BaseLight
{
    Vector3f Color;
    float AmbientIntensity;
    float DiffuseIntensity;

    BaseLight()
    {
        Color = Vector3f(0.0f, 0.0f, 0.0f);
        AmbientIntensity = 0.0f;
        DiffuseIntensity = 0.0f;
    }
};

struct DirectionalLight : public BaseLight
{
    Vector3f Direction;

    DirectionalLight()
    {
        Direction = Vector3f(0.0f, 0.0f, 0.0f);
    }
};

struct PointLight : public BaseLight
{
    Vector3f Position;

    struct
    {
        float Constant;
        float Linear;
        float Exp;
    } Attenuation;

    PointLight()
    {
        Position = Vector3f(0.0f, 0.0f, 0.0f);
        Attenuation.Constant = 1.0f;
        Attenuation.Linear = 0.0f;
        Attenuation.Exp = 0.0f;
    }
};

struct SpotLight : public PointLight
{
    Vector3f Direction;
    float Cutoff;

    SpotLight()
    {
        Direction = Vector3f(0.0f, 0.0f, 0.0f);
        Cutoff = 0.0f;
    }
};



class LightingTechnique : public Technique
{

private:

    GLuint WVPLocation;
    GLuint WorldMatrixLocation;
    GLuint samplerLocation;

    GLuint eyeWorldPosLocation;
    GLuint matSpecularIntensityLocation;
    GLuint matSpecularPowerLocation;

    GLuint numPointLightsLocation;
    GLuint numSpotLightsLocation;

    GLuint LightWVPLocation;
    GLuint shadowMapLocation;

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Direction;
    } dirLightLocation;

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } pointLightsLocation[MAX_POINT_LIGHTS];

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        GLuint Direction;
        GLuint Cutoff;
        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } spotLightsLocation[MAX_SPOT_LIGHTS];

public:

    LightingTechnique() { }

    bool Init()
    {
        if (!Technique::Init()) return false;

        if (!AddShader(GL_VERTEX_SHADER, vertex_LT)) return false;

        if (!AddShader(GL_FRAGMENT_SHADER, fragment_LT)) return false;

        if (!Finalize()) return false;

        WVPLocation = GetUniformLocation("gWVP");
        WorldMatrixLocation = GetUniformLocation("gWorld");
        samplerLocation = GetUniformLocation("gSampler");

        eyeWorldPosLocation = GetUniformLocation("gEyeWorldPos");
        dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Base.Color");
        dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
        dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
        dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");

        matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
        matSpecularPowerLocation = GetUniformLocation("gSpecularPower");

        numPointLightsLocation = GetUniformLocation("gNumPointLights");
        numSpotLightsLocation = GetUniformLocation("gNumSpotLights");

        LightWVPLocation = GetUniformLocation("gLightWVP");
        shadowMapLocation = GetUniformLocation("gShadowMap");

        if (dirLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            WVPLocation == INVALID_UNIFORM_LOCATION ||
            WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
            samplerLocation == INVALID_UNIFORM_LOCATION ||
            eyeWorldPosLocation == INVALID_UNIFORM_LOCATION ||
            dirLightLocation.Color == INVALID_UNIFORM_LOCATION ||
            dirLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            dirLightLocation.Direction == INVALID_UNIFORM_LOCATION ||
            matSpecularIntensityLocation == INVALID_UNIFORM_LOCATION ||
            matSpecularPowerLocation == INVALID_UNIFORM_LOCATION ||
            numPointLightsLocation == INVALID_UNIFORM_LOCATION ||
            numSpotLightsLocation == INVALID_UNIFORM_LOCATION) {
            return false;
        }

        for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(pointLightsLocation); i++)
        {
            char Name[128];
            memset(Name, 0, sizeof(Name));
            snprintf(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
            pointLightsLocation[i].Color = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
            pointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Position", i);
            pointLightsLocation[i].Position = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
            pointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
            pointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
            pointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
            pointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

            if (pointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
                pointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
                return false;
            }
        }

        for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(spotLightsLocation); i++)
        {
            char Name[128];
            memset(Name, 0, sizeof(Name));
            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
            spotLightsLocation[i].Color = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
            spotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
            spotLightsLocation[i].Position = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
            spotLightsLocation[i].Direction = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
            spotLightsLocation[i].Cutoff = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
            spotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
            spotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
            spotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

            snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
            spotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

            if (spotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
                spotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
                return false;
            }
        }

        return true;
    }

    // calculated from the position of the light source
    void SetLightWVP(const Matrix4f& LightWVP)
    {
        glUniformMatrix4fv(LightWVPLocation, 1, GL_TRUE, (const GLfloat*)LightWVP.m);
    }

    void SetShadowMapTextureUnit(unsigned int TextureUnit)
    {
        glUniform1i(shadowMapLocation, TextureUnit);
    }

    void SetWVP(const Matrix4f& WVP)
    {
        glUniformMatrix4fv(WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
    }

    void SetWorldMatrix(const Matrix4f& WorldInverse)
    {
        glUniformMatrix4fv(WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
    }

    void SetTextureUnit(unsigned int TextureUnit)
    {
        glUniform1i(samplerLocation, TextureUnit);
    }

    void SetDirectionalLight(const DirectionalLight& Light)
    {
        glUniform3f(dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
        glUniform1f(dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
        Vector3f Direction = Light.Direction;
        Direction.Normalize();
        glUniform3f(dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
    }

    void SetEyeWorldPos(const Vector3f& EyeWorldPos)
    {
        glUniform3f(eyeWorldPosLocation, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
    }

    void SetMatSpecularIntensity(float Intensity)
    {
        glUniform1f(matSpecularIntensityLocation, Intensity);
    }

    void SetMatSpecularPower(float Power)
    {
        glUniform1f(matSpecularPowerLocation, Power);
    }

    void SetPointLights(unsigned int NumLights, const PointLight* pLights)
    {
        glUniform1i(numPointLightsLocation, NumLights);

        for (unsigned int i = 0; i < NumLights; i++)
        {
            glUniform3f(pointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
            glUniform1f(pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
            glUniform1f(pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
            glUniform3f(pointLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
            glUniform1f(pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
            glUniform1f(pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
            glUniform1f(pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
        }
    }

    void SetSpotLights(unsigned int NumLights, const SpotLight* pLights)
    {
        glUniform1i(numSpotLightsLocation, NumLights);

        for (unsigned int i = 0; i < NumLights; i++)
        {
            glUniform3f(spotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
            glUniform1f(spotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
            glUniform1f(spotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
            glUniform3f(spotLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
            Vector3f Direction = pLights[i].Direction;
            Direction.Normalize();
            glUniform3f(spotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
            glUniform1f(spotLightsLocation[i].Cutoff, cosf(glm::radians(pLights[i].Cutoff)));
            glUniform1f(spotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
            glUniform1f(spotLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
            glUniform1f(spotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
        }
    }
};


#endif	/* LIGHTING_TECHNIQUE_H */