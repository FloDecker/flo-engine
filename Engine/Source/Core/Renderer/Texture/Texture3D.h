#pragma once
#include <vec3.hpp>

#include "Texture.h"

class Texture3D : public Texture
{
private:
    unsigned int _texture;
    bool _initalized = false;

    unsigned short int* _data;

    unsigned int width_ = 0;
    unsigned int height_ = 0;
    unsigned int depth_ = 0;

    float step_size_ = 0;
    glm::vec3 voxel_field_lower_left_ = glm::vec3(0.0);
    glm::vec3 voxel_field_upper_right_ = glm::vec3(0.0);

public:
    void initialize();
    void use(unsigned int textureUnit) override;
    unsigned int getTexture();
    void initalize_as_voxel_data(glm::vec3 voxel_field_lower_left,
                                 glm::vec3 voxel_field_upper_right,
                                 float step_size,
                                 unsigned int width,
                                 unsigned int height,
                                 unsigned int depth);
    void write_to_voxel_field(unsigned short r, unsigned short g, unsigned short b, unsigned short a,
                              unsigned int pos_width, unsigned int pos_height, unsigned int pos_depth);

    //getter
    unsigned int get_width() const { return width_; }
    unsigned int get_height() const { return height_; }
    unsigned int get_depth() const { return depth_; }
    float get_step_size() const { return step_size_; }
    glm::vec3 get_voxel_field_lower_left() const { return voxel_field_lower_left_; }
    glm::vec3 get_voxel_field_upper_right() const { return voxel_field_upper_right_; }
};
