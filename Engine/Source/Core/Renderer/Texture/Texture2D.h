#pragma once
#include <string>
#include <iostream>

#include "Texture.h"

class Texture2D: public Texture
{
private:
    unsigned int _texture;
    bool _loaded = false;
    std::string *_path;

    //dimentions
    int _width = 0;
    int _height = 0;
    int _channleAmount = 0;
public:
    void initialize(unsigned char *data);
    void use(unsigned int textureUnit) override;
    unsigned int getTexture();
    void loadFromDisk(std::string* path);
};
