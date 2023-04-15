#include "Texture.h"
#include <GL/glew.h>

void Texture::initialize(unsigned char *data)
{
    glGenTextures(1,&_texture);
    glBindTexture(GL_TEXTURE_2D,_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,_width,_height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

unsigned Texture::getTexture()
{
    return  0;
}


void Texture::setImageDimension(unsigned width, unsigned height)
{
    this->_width = width;
    this->_height = height;
}

void Texture::loadFromDisk(std::string *path)
{
    if (this->_width == 0 || this->_height == 0)
    {
        std::cerr<<"image dimension of image are unknown or one of them was set to 0"<<std::endl;
        return;
    }
    if (_channleAmount == 0)
    {
        std::cerr<<"no channel amount was defined"<<std::endl;
        return;
    }
    this->_path = path;
    //TODO: finish texture loading
    //unsigned char *data  = stbi_load(_path->c_str(),&_width,&_height,&_channleAmount,0);
    //if(!data)
    //{
    //    std::cerr<<"couldn't load image " << _path << std::endl;
    //}
    //this->initialize(data);
    //stbi_image_free(data);
}
