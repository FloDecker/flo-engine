#include "Voxelizer.h"

#include <vec3.hpp>

#include "../SceneContext.h"
#include "../DebugPrimitives/Cube3D.h"
#include "../RayCast.h"

Voxelizer::Voxelizer(GlobalContext* global_context, SceneContext* scene_context): Object3D(global_context)
{
    scene_context_=scene_context;
}

void Voxelizer::recalculate()
{
    auto transform_global = getGlobalTransform();
    float distance_x = glm::distance(transform_global * glm::vec4(1, 0, 0, 1),
                                     transform_global * glm::vec4(-1, 0, 0, 1));
    float distance_y = glm::distance(transform_global * glm::vec4(0, 1, 0, 1),
                                     transform_global * glm::vec4(0, -1, 0, 1));
    float distance_z = glm::distance(transform_global * glm::vec4(0, 0, 1, 1),
                                     transform_global * glm::vec4(0, 0, -1, 1));

    glm::vec3 vec_test = glm::vec3(distance_x/ voxel_precision,distance_y/ voxel_precision,distance_z/ voxel_precision);
    glm::vec3 vec_test_half = glm::vec3(0.5*distance_x/ voxel_precision,0.5*distance_y/ voxel_precision,0.5*distance_z/ voxel_precision);
    float length_test_vec = glm::length(vec_test);
    for (float x = -1; x < 1; x += 2.0f / static_cast<float>(voxel_precision))
    {
        for (float y = -1; y <1; y += 2.0f / static_cast<float>(voxel_precision))
        {
            for (float z = -1; z < 1; z += 2.0f / static_cast<float>(voxel_precision))
            {
                auto position_local = glm::vec3(x,y,z);
                auto position_global_4 = getGlobalTransform() * glm::vec4(position_local,1.0);
                auto position_global_3 = glm::vec3(position_global_4);
                auto c = new Cube3D(global_context_);
                this->addChild(c);
                c->setScale(0.02);
                c->color = {0,1,0};
                c->set_position_global(position_global_3);
                
                RayCastHit h = RayCast::ray_cast_editor(scene_context_,position_global_3-vec_test_half, -vec_test_half );
                if(h.hit)
                {
                    auto c = new Cube3D(global_context_);
                    this->addChild(c);
                    c->setScale(0.02);
                    c->set_position_global(h.hit_normal_world_space);
                }

            }
        }
    }
}
