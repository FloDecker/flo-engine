#pragma once
#include <string>
#include <iostream>

class Texture
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
    void use(unsigned int textureUnit);
    unsigned int getTexture();
    void loadFromDisk(std::string* path);
};
