#include "Texture3D.h"

#include <iostream>
#include <GL/glew.h>

void Texture3D::initialize()
{

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_3D, _texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA4, _width, _height, _depth, 0,GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4, _data);
    glGenerateMipmap(GL_TEXTURE_3D);



}

void Texture3D::use(unsigned int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, _texture);
    glBindSampler(GL_TEXTURE0 + textureUnit, textureUnit);
}

unsigned Texture3D::getTexture()
{
    return _texture;
}

void Texture3D::initalize_as_voxel_data(glm::vec3 voxel_field_lower_left, glm::vec3 voxel_field_upper_right,
                                        unsigned int step_size, unsigned int width, unsigned int height,
                                        unsigned int depth)
{
    _width = width; //x
    _height = height; //y
    _depth = depth; //z

    //allocate memory for 3d texture
    this->_data = static_cast<unsigned short*>(malloc(width * 2 * height * 2 * depth * 2));
    _initalized = true;
}

void Texture3D::write_to_voxel_field(unsigned short r, unsigned short g, unsigned short b, unsigned short a, unsigned int pos_width,
                                     unsigned int pos_height, unsigned int pos_depth)
{
    if (!_initalized)
    {
        std::cerr << "cant write to uninitialized voxel field\n";
        return;
    }
    if (pos_width > _width || pos_depth > _depth || pos_height > _height)
    {
        std::cerr << "texel with coordinates: w: " << pos_width << " h: " << pos_height << " d: " << pos_depth << " is out of bounds\n";
        return;
    }
    unsigned short color_data_r = r;
    unsigned short color_data_g = g << 4;
    unsigned short color_data_b = b << 8;
    unsigned short color_data_a = a << 12;
    
    unsigned short color_data = (color_data_r|color_data_g|color_data_b|color_data_a);
    
    _data[2*(_width*_height*pos_depth+_width*pos_height+pos_width)] = color_data;
    
}
