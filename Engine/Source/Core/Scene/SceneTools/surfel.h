#pragma once
#include <vec3.hpp>

struct surfel
{
    glm::vec3 mean;
    glm::vec3 normal;
    glm::vec3 diffuse_irradiance;
    float radius;
    
    //mapping information on where the surfel is stored in GPU memory
    //it's a vector for the case of the surfel crossing a buckets edge and therefor being in multiple buckets
    std::vector<unsigned int> *surfel_in_gpu_memory = new std::vector<unsigned int>();

    //how often this surfel was sampled to obtain the current color 
    int samples = 0;
};
