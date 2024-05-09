#pragma once
#include <vec3.hpp>

#include "Texture.h"

class Texture3D: public Texture
{
private:
    unsigned int _texture;
    bool _initalized = false;

    //dimentions
    unsigned int _width = 0;
    unsigned int _height = 0;
    unsigned int _depth = 0;

    unsigned short* _data;

public:
    void initialize();
    void use(unsigned int textureUnit) override;
    unsigned int getTexture();
    void initalize_as_voxel_data(glm::vec3 voxel_field_lower_left,
                                 glm::vec3 voxel_field_upper_right,
                                 unsigned int step_size,
                                 unsigned int width,
                                 unsigned int height,
                                 unsigned int depth);
    void write_to_voxel_field(unsigned short r, unsigned short g, unsigned short b, unsigned short a,
        unsigned int pos_width,unsigned int pos_height,unsigned int pos_depth);
}; 
