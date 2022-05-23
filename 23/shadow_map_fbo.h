#include <GL/glew.h>
#include <stdio.h>

//framebuffer object
class ShadowMapFBO
{
public:
    ShadowMapFBO()
    {
        m_fbo = 0;
        m_shadowMap = 0;
    }

    ~ShadowMapFBO()
    {
        if (m_fbo != 0) {
            glDeleteFramebuffers(1, &m_fbo);
        }

        if (m_shadowMap != 0) {
            glDeleteFramebuffers(1, &m_shadowMap);
        }
    }

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight)
    {
        // create fbo
        glGenFramebuffers(1, &m_fbo);

        // create a texture which is a shadow map
        glGenTextures(1, &m_shadowMap);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // bind texture and fbo
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
            m_shadowMap, 0);

        // do not render into the color buffer
        glDrawBuffer(GL_NONE);

        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (Status != GL_FRAMEBUFFER_COMPLETE) {
            printf("FB error, status: 0x%x\n", Status);
            return false;
        }

        return true;
    }

    // switch to the fbo (as we have to switch between shadow map and standart buffer)
    void BindForWriting() // first pass 
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    }

    void BindForReading(GLenum TextureUnit) // second pass
    {
        // bind the texture object, not fbo
        glActiveTexture(TextureUnit);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    }

private:
    GLuint m_fbo;
    GLuint m_shadowMap;
};