#include "Voxelizer.h"

#include <chrono>
#include <vec3.hpp>

#include "../../SceneContext.h"
#include "../../DebugPrimitives/Cube3D.h"
#include "../../RayCast.h"
#include "../../DebugPrimitives/Line3D.h"
#define FLOATING_POINT_ACCEPTANCE 0.02

Voxelizer::Voxelizer(GlobalContext* global_context, SceneContext* scene_context): AbstractVoxelizer(global_context,scene_context){};

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

    calculate_area_filled_recursive(scene_context_, upper_right_corner, lower_left_corner,
                                    {distance_x, distance_y, distance_z}, {0,0,0});

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "voxel build time: " << elapsed_seconds.count() << "s\n";
    std::cout << "build " << voxel_positions_.size() << " voxels\n";
}

void Voxelizer::load_into_voxel_texture(Texture3D* texture_3d)
{
    auto transform_global_inverse = glm::inverse(getGlobalTransform());

    
    for (auto v : voxel_positions_)
    {
        glm::vec3 object_space = transform_global_inverse*glm::vec4(v,1.0);
        object_space*=0.5f;
        object_space+=glm::vec3(0.5);
        texture_3d->write_to_voxel_field_float(15,15,15,15,object_space.x,object_space.y,object_space.z);
    }
}
void Voxelizer::load_into_voxel_texture_df(Texture3D* texture_3d)
{
    auto transform_global = getGlobalTransform();
    glm::vec3 upper_right_corner = glm::round(transform_global * glm::vec4(1, 1, 1, 1));
    glm::vec3 lower_left_corner = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));
    unsigned int distance_x = static_cast<int>(abs(upper_right_corner.x - lower_left_corner.x)) * voxel_precision;
    unsigned int distance_y = static_cast<int>(abs(upper_right_corner.y - lower_left_corner.y)) * voxel_precision;
    unsigned int distance_z = static_cast<int>(abs(upper_right_corner.z - lower_left_corner.z)) * voxel_precision;

    
    int level_set_size = distance_x * distance_y * distance_z;
    
    int *level_set =new int[level_set_size]();  //init all entrys as -1
    memset(level_set, -1, sizeof(int) * level_set_size);

    if(!level_set)
    {
        std::cerr << "steps from zero array matrix memory couldn't be allocated";
        return; 
    }

    int current_level = 0;
    std::vector<glm::i16vec3> current_set = zero_level_set;

    constexpr glm::i16vec3 directions[] = {
        glm::i16vec3(1, 1, 1),
        glm::i16vec3(1, 0, 1),
        glm::i16vec3(1, -1, 1),
        glm::i16vec3(0, 1, 1),
        glm::i16vec3(0, 0, 1),
        glm::i16vec3(0, -1, 1),
        glm::i16vec3(-1, 1, 1),
        glm::i16vec3(-1, 0, 1),
        glm::i16vec3(-1, -1, 1),

        glm::i16vec3(1, 1, 0),
        glm::i16vec3(1, 0, 0),
        glm::i16vec3(1, -1, 0),
        glm::i16vec3(0, 1, 0),
        glm::i16vec3(0, -1, 0),
        glm::i16vec3(-1, 1, 0),
        glm::i16vec3(-1, 0, 0),
        glm::i16vec3(-1, -1, 0),

        glm::i16vec3(1, 1, -1),
        glm::i16vec3(1, 0, -1),
        glm::i16vec3(1, -1, -1),
        glm::i16vec3(0, 1, -1),
        glm::i16vec3(0, 0, -1),
        glm::i16vec3(0, -1, -1),
        glm::i16vec3(-1, 1, -1),
        glm::i16vec3(-1, 0, -1),
        glm::i16vec3(-1, -1, -1),
    };

    std::vector<glm::i16vec3> next_set;
    std::cout<<"Building level set for distance function :";
    while (!current_set.empty())
    {
        next_set.clear();
        std::cout<<current_level<<" | ";
        
        for (auto position : current_set)
        {
            for (auto offset : directions)
            {
                glm::i16vec3 test_pos = position + offset;
                if (test_pos.x >= 0 && test_pos.y >= 0 && test_pos.z >= 0
                    && test_pos.x < distance_x && test_pos.y < distance_y && test_pos.z < distance_z)
                {
                    //if inside boundaries
                    int level_at_pos = level_set[distance_x * distance_y * test_pos.z + distance_x * test_pos.y + test_pos.x];
                    if (level_at_pos == -1)
                    {
                        level_set[distance_x * distance_y * test_pos.z + distance_x * test_pos.y + test_pos.x] = current_level + 1;
                        next_set.push_back(test_pos); // insert value into set for next interation;
                        //TODO: change datatype of 3d texture to fit more distances 
                        texture_3d->write_to_voxel_field(0,0,0,(current_level + 1 < 14) ? current_level + 1:14,test_pos.x,test_pos.y,test_pos.z);


                        //just for debugging
                        /*
                        if (current_level == 3)
                        {
                            auto transform_global = getGlobalTransform();
                            glm::vec3 ws_lower_left = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));
                        
                            float step_size = 1.0f / static_cast<float>(voxel_precision);
                            auto ws_pos = ws_lower_left + static_cast<glm::vec3>(test_pos) * step_size + step_size * 0.5f;
                        
                            auto c = new Cube3D(global_context_);

                            
                            this->addChild(c);
                            c->setScale(glm::vec3(0.01,0.01,0.01));
                            float interpol;
                            interpol = static_cast<float>(current_level) / 4.0f;

                            c->color = glm::vec3(1, 0, 0)*interpol+glm::vec3(0, 1, 0)*(1.0f-interpol);
                            c->set_position_global(ws_pos);
                        }
*/
                        
                        
                    }
                }
            }
        }
        
        current_level+=1; //increment current level set
        current_set = next_set; //copy next set positions to new set;

        
    }
    std::cout<<"\n";

    
    free(level_set);

    
}


void Voxelizer::calculate_area_filled_recursive(SceneContext* scene_context, glm::vec3 ws_upper_right,
                                                glm::vec3 ws_lower_left, glm::i16vec3 voxel_upper_right,
                                                glm::i16vec3 voxel_lower_left)
{
    float step_size = 1.0f / static_cast<float>(voxel_precision);

    int distance_x = abs(voxel_upper_right.x - voxel_lower_left.x);
    int distance_y = abs(voxel_upper_right.y - voxel_lower_left.y);
    int distance_z = abs(voxel_upper_right.z - voxel_lower_left.z);

    //if all distances are the same we can do a distance check from the center
    if (distance_x == distance_y && distance_x == distance_z)
    {
        glm::vec3 center = ws_lower_left + (static_cast<glm::vec3>(voxel_lower_left) + static_cast<glm::vec3>(voxel_upper_right)) *
            step_size * 0.5f;

        //float radius = glm::length(glm::vec3(step_size)) * 0.5f;
        float radius = glm::length(glm::vec3(step_size))*distance_x*0.5;
        
        if (RayCast::scene_geometry_proximity_check(scene_context_, center,radius))
        {
            //raycast hit and smallest step size reached
            if (distance_x == 1 && distance_y == 1 && distance_z == 1)
            {

                auto ws_pos = ws_lower_left + static_cast<glm::vec3>(voxel_lower_left) * step_size + step_size * 0.5f;

                //auto c = new Cube3D(global_context_);
                //this->addChild(c);
                //c->setScale(distance_x * (step_size / this->get_scale().x), distance_x * (step_size / this->get_scale().y),
                //            distance_x * (step_size / this->get_scale().z));
                //c->color = {0, 1, 0};
                //c->set_position_global(ws_pos);
                
                voxel_positions_.push_back(ws_pos);
                zero_level_set.push_back(voxel_lower_left);
                return;
            } else
            {

                //auto c = new Cube3D(global_context_);
                //this->addChild(c);
                //c->setScale(distance_x * (step_size / this->get_scale().x), distance_x * (step_size / this->get_scale().y),
                //            distance_x * (step_size / this->get_scale().z));
                //c->color = {1, 0, 0};
                //c->set_position_global(center);
                
                int distance = (distance_x + 1) / 2;
                glm::i16vec3 offset_of_chunk = glm::i16vec3(distance);
                for (int x = 0; x <= 1; x++)
                {
                    for (int y = 0; y <= 1; y++)
                    {
                        for (int z = 0; z <= 1; z++)
                        {
                            glm::i16vec3 offset = {x * distance, y * distance, z * distance};
                            calculate_area_filled_recursive(scene_context, ws_upper_right,
                                                            ws_lower_left, voxel_lower_left + offset + offset_of_chunk,
                                                            voxel_lower_left + offset);
                        }
                    }
                }
            }
            //countinue seperating
        }
        else
        {
            // no hit in chunk, abort
            //TEMP FOR TEST
            return;
        }
    }


    if (distance_x <= distance_y && distance_x <= distance_z) // x is the smallest
    {

    }
    if (distance_y <= distance_x && distance_y <= distance_z) // y is the smallest
    {
    }
    if (distance_z <= distance_x && distance_z <= distance_y) // z is the smallest
    {
    }
}
