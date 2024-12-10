//
// Created by flode on 28/02/2023.
//

#ifndef ENGINE_RENDERABLE_H
#define ENGINE_RENDERABLE_H
#include <mat3x3.hpp>
#include <vec3.hpp>
#include <ext/matrix_transform.hpp>

class primitive {
private:
    bool isLoaded = false;
    bool mesh_calculation_ran = false;

    void run_mesh_calculations();


public:
    virtual int load() = 0;
    virtual int draw() = 0;

 
    
};



#endif //ENGINE_RENDERABLE_H
