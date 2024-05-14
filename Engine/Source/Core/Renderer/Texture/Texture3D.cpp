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
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA4, width_, height_, depth_, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, _data);
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
                                        float step_size, unsigned int width, unsigned int height,
                                        unsigned int depth)
{
    //allocate memory for 3d texture
    this->_data = static_cast<unsigned short int*>(calloc(1, width * 2 * height * 2 * depth * 2));
    if(!this->_data)
    {
        std::cerr<<"couldn't allocate memory for 3D texture\n";
        return;
    }
    width_ = width; //x
    height_ = height; //y
    depth_ = depth; //z

    step_size_ = step_size;
    voxel_field_lower_left_ = voxel_field_lower_left;
    voxel_field_upper_right_ = voxel_field_upper_right;
    _initalized = true;
}

void Texture3D::write_to_voxel_field(unsigned short r, unsigned short g, unsigned short b, unsigned short a,
                                     unsigned int pos_width,
                                     unsigned int pos_height, unsigned int pos_depth)
{
    if (!_initalized)
    {
        std::cerr << "cant write to uninitialized voxel field\n";
        return;
    }
    if (pos_width > width_ || pos_depth > depth_ || pos_height > height_)
    {
        std::cerr << "texel with coordinates: w: " << pos_width << " h: " << pos_height << " d: " << pos_depth <<
            " is out of bounds\n";
        return;
    }
    unsigned short int color_data_r = r << 12;
    unsigned short int color_data_g = g << 8;
    unsigned short int color_data_b = b << 4;
    unsigned short int color_data_a = a;

    unsigned short int color_data = (color_data_r | color_data_g | color_data_b | color_data_a);

    _data[(width_ * height_ * pos_depth + width_ * pos_height + pos_width)] = color_data;
}
