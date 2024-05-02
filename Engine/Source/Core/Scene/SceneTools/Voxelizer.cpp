#include "Voxelizer.h"

#include <chrono>
#include <vec3.hpp>

#include "../SceneContext.h"
#include "../DebugPrimitives/Cube3D.h"
#include "../RayCast.h"
#include "../DebugPrimitives/Line3D.h"
#define FLOATING_POINT_ACCEPTANCE 0.02
Voxelizer::Voxelizer(GlobalContext* global_context, SceneContext* scene_context): Object3D(global_context)
{
    scene_context_ = scene_context;
}

void Voxelizer::recalculate()
{

    auto start = std::chrono::system_clock::now();

    auto transform_global = getGlobalTransform();
    glm::vec3 upper_right_corner = glm::round(transform_global * glm::vec4(1, 1, 1, 1));
    glm::vec3 lower_left_corner = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));

    unsigned int distance_x = static_cast<int>(abs(upper_right_corner.x - lower_left_corner.x)) * voxel_precision;
    unsigned int distance_y = static_cast<int>(abs(upper_right_corner.y - lower_left_corner.y)) * voxel_precision;
    unsigned int distance_z = static_cast<int>(abs(upper_right_corner.z - lower_left_corner.z)) * voxel_precision;

    if (upper_right_corner.x - lower_left_corner.x < 1.0)
    {
        std::cout << "needs an extension in x of at least 1\n";
    }
    if (upper_right_corner.y - lower_left_corner.y < 1.0)
    {
        std::cout << "needs an extension in y of at least 1\n";
    }
    if (upper_right_corner.z - lower_left_corner.z < 1.0)
    {
        std::cout << "needs an extension in z of at least 1\n";
    }

    float step_size = 1.0f / static_cast<float>(voxel_precision);
    glm::vec3 half_vector = glm::vec3(0.5f * step_size);

    for (unsigned int x = 0; x < distance_x; x += 1)
    {
        for (unsigned int y = 0; y < distance_y; y += 1)
        {
            for (unsigned int z = 0; z < distance_z; z += 1)
            {
                auto position_global = lower_left_corner +
                    glm::vec3(static_cast<float>(x) * step_size,
                              static_cast<float>(y) * step_size,
                              static_cast<float>(z) * step_size);
                if (RayCast::scene_geometry_proximity_check(scene_context_,position_global+half_vector,step_size*0.5f))
                {
                    auto c = new Cube3D(global_context_);
                    this->addChild(c);
                    c->setScale(step_size/this->get_scale().x,step_size/this->get_scale().y,step_size/this->get_scale().z);
                    c->color = {0, 1, 0};
                    c->set_position_global(position_global);
                }

            }
        }
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "voxel build time: " << elapsed_seconds.count() << "s\n";
}

bool Voxelizer::proble_single_chunc(SceneContext* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
                                    float overlap_percentage)
{


    //float length = glm::length(vec_1);
    //RayCastHit h = RayCast::ray_cast_editor(scene_context_, position_global_3, -vec_test_half, false);
    //new Line3D(scene_context_->get_root(), position_global_3, position_global_3 - vec_test_half,
    //           global_context_);
    //if (h.hit)
    //{
    //    auto c = new Cube3D(global_context_);
    //    this->addChild(c);
    //    c->setScale(0.02);
    //    c->set_position_global(h.hit_world_space);
    //}
    return false;
}
