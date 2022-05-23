#include <GL/glew.h>
#include <stdio.h>

class ShadowMapFBO
{
public:
    ShadowMapFBO()
    {
        fbo = 0;
        shadowMap = 0;
    }

    ~ShadowMapFBO()
    {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
        if (shadowMap != 0) glDeleteFramebuffers(1, &shadowMap);
    }

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight)
    {
        glGenFramebuffers(1, &fbo);

        glGenTextures(1, &shadowMap);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
            shadowMap, 0);

        glDrawBuffer(GL_NONE);

        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (Status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("FB error, status: 0x%x\n", Status);
            return false;
        }

        return true;
    }

    void BindForWriting()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    }

    void BindForReading(GLenum TextureUnit)
    {
        glActiveTexture(TextureUnit);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
    }

private:
    GLuint fbo;
    GLuint shadowMap;
};
