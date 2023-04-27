#include "Texture.h"
#include <GL/glew.h>
#include <stb_image.h>
void Texture::initialize(unsigned char *data)
{
    glGenTextures(1,&_texture);
    glBindTexture(GL_TEXTURE_2D,_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,_width,_height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
   
}

void Texture::use(unsigned int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D,_texture);
    glBindSampler(GL_TEXTURE0 + textureUnit,textureUnit );
}

unsigned Texture::getTexture()
{
    return  _texture;
}

void Texture::loadFromDisk(std::string *path)
{
    this->_path = path;
    unsigned char *data  = stbi_load(_path->c_str(),&_width,&_height,&_channleAmount,0);
    if(!data)
    {
        std::cerr<<"couldn't load image " << _path << std::endl;
    }
    this->initialize(data);
    stbi_image_free(data);
}
