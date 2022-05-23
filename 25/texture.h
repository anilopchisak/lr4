#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>

#include <GL/glew.h>
#include <Magick++.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    bool Load();

    void Bind(GLenum TextureUnit);

private:
    std::string fileName;
    GLenum textureTarget;
    GLuint textureObj;
    Magick::Image* pImage;
    Magick::Blob blob;
};


#endif	/* TEXTURE_H */

