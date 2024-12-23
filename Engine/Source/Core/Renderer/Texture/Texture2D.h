#pragma once
#include <string>
#include <iostream>

#include "Texture.h"

class Texture2D: public Texture
{
private:
    unsigned int _texture;
    bool initialized_ = false;
    std::string *_path;
    void initialize_from_data(unsigned char *data);

    //dimentions
    int width_ = 0;
    int height_ = 0;
    int _channleAmount = 0;
public:
    void initialize_as_depth_map_render_target(unsigned int width,  unsigned int height );
    void use(unsigned int textureUnit) override;
    unsigned int get_texture() const;
    void loadFromDisk(std::string* path);

    [[nodiscard]] int width() const
    {
        return width_;
    }

    [[nodiscard]] int height() const
    {
        return height_;
    }
};
