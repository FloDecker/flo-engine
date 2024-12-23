#include "Texture2D.h"
#include <GL/glew.h>
#include <stb_image.h>
void Texture2D::initialize_from_data(unsigned char *data)
{
    if (initialized_)
    {
        std::cerr << "Texture2D already initialized\n";
    }
    if (width_ == 0 || height_ == 0 )
    {
        std::cerr << "Texture2D width or height cannot be zero\n";
    }
    glGenTextures(1,&_texture);
    glBindTexture(GL_TEXTURE_2D,_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width_,height_,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    initialized_ = true;
   
}

void Texture2D::initialize_as_depth_map_render_target(const unsigned int width, const unsigned int height )
{
    if (initialized_)
    {
        std::cerr << "Texture2D already initialized\n";
    }
    width_ = width;
    height_ = height;

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    initialized_ = true;

}

void Texture2D::use(unsigned int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D,_texture);
    glBindSampler(GL_TEXTURE0 + textureUnit,textureUnit );
}

unsigned Texture2D::get_texture() const
{
    return  _texture;
}

void Texture2D::loadFromDisk(std::string *path)
{
    this->_path = path;
    unsigned char *data  = stbi_load(_path->c_str(),&width_,&height_,&_channleAmount,0);
    if(!data)
    {
        std::cerr<<"couldn't load image " << _path << std::endl;
    }
    this->initialize_from_data(data);
    stbi_image_free(data);
}
