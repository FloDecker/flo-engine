//
// Created by flode on 28/02/2023.
//

#ifndef ENGINE_RENDERABLE_H
#define ENGINE_RENDERABLE_H
class Renderable {
private:
    bool isLoaded = false;
public:
    virtual int load() = 0;
    virtual int draw() = 0;
};


#endif //ENGINE_RENDERABLE_H
