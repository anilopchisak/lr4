#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>
#include <GL/glew.h>
#include <iostream>
#include <Magick++.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName)
    {
        textureTarget = TextureTarget;
        fileName = FileName;
        pImage = NULL;
    }

    bool Load()
    {
        try {
            pImage = new Magick::Image(fileName);
            pImage->write(&blob, "RGBA");
        }
        catch (Magick::Error& Error) {
            std::cout << "Error loading texture '" << fileName << "': " << Error.what() << std::endl;
            return false;
        }

        glGenTextures(1, &textureObj);
        glBindTexture(textureTarget, textureObj);
        glTexImage2D(textureTarget, 0, GL_RGB, pImage->columns(), pImage->rows(), -0.5, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());
        glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return true;
    }

    void Bind(GLenum TextureUnit)
    {
        glActiveTexture(TextureUnit);
        glBindTexture(textureTarget, textureObj);
    }


private:
    std::string fileName;
    GLenum textureTarget;
    GLuint textureObj;
    Magick::Image* pImage;
    Magick::Blob blob;
};


#endif	/* TEXTURE_H */
