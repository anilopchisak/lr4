#include <iostream>
#include "texture.h"

Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{
    textureTarget = TextureTarget;
    fileName      = FileName;
    pImage        = NULL;
}

bool Texture::Load()
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

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(textureTarget, textureObj);
}
