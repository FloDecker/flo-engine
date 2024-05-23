#include "SceneContext.h"

#include <chrono>
#include <iostream>

#include "Collider.h"
#include "DebugPrimitives/Cube3D.h"


SceneContext::SceneContext(GlobalContext* global_context, Object3D* scene_root)
{
    global_context_ = global_context;
    scene_root_ = scene_root;

    //get ids of engine defined tags
    engine_light_point_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_LIGHT_POINT");
    engine_collider_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_COLLIDER");
    recalculate_from_root();
}


void SceneContext::recalculate_at(Object3D* parent)
{
    //sort objets by their tag
    if (parent->has_tag(engine_light_point_id_))
    {
        this->scenePointLights.insert(dynamic_cast<PointLight*>(parent));
    }

    if (parent->has_tag(engine_collider_id_))
    {
        this->sceneColliders.push_back(dynamic_cast<Collider*>(parent));
    }

    const auto children = parent->get_children();
    for (Object3D* child : children)
    {
        this->recalculate_at(child);
    }
}

void SceneContext::recalculate_from_root()
{
    recalculate_at(scene_root_);
    calculateColliderBoundingBoxes();
    calcualteSceneTree();

}


void SceneContext::calculateColliderBoundingBoxes()
{
    for (auto collider : sceneColliders)
    {
        collider->calculate_world_space_bounding_box();

        //TODO REMOVE THIS TEST
        auto c = new Cube3D(global_context_);
        scene_root_->addChild(c);
        c->setScale(BoundingBoxHelper::get_scale_of_bb(&collider->bounding_box));
        c->color = {0, 0, 1};
        c->set_position_global(BoundingBoxHelper::get_center_of_bb(&collider->bounding_box));
        ////////////////////TEST END //////////////////////////
    }
}

void SceneContext::calcualteSceneTree()
{
    if (sceneColliders.empty())return;

    auto start = std::chrono::system_clock::now();

    //allocate memory for kd tree
    //sceneColliders.size() for the leaf nodes and sceneColliders.size()-1 for the 
    axis_aligned_bb_tree_ = static_cast<kdTreeElement*>(calloc(sceneColliders.size()*2-1 ,sizeof(kdTreeElement)));

    //vector that holds the positions in axis_aligned_bb_tree_ that the matrix represents e.g at matrix x,y = 4 represents
    //the bounding box positioned at pos 8 in axis_aligned_bb_tree_ -> matrix_bb_tree_map[4] = 8
    std::vector<int> matrix_bb_tree_map;
    matrix_bb_tree_map.resize(sceneColliders.size());

    //insert the objects into the leaf nodes
    for (unsigned int x = 0; x < sceneColliders.size(); x++)
    {
        auto temp = kdTreeElement{
            -1,
            static_cast<int>(x),
            sceneColliders.at(x)->bounding_box
        };
        axis_aligned_bb_tree_[x] = temp;
        matrix_bb_tree_map[x] = x;
    }
    
    StructBoundingBox temp_bb;
    BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &sceneColliders.at(0)->bounding_box, &sceneColliders.at(1)->bounding_box);
    float smallest_box = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
    glm::i32vec2 next_merge = {0,1};
    
    //calculate distance matrix
    std::vector<std::vector<float>> distance_matrix;
    distance_matrix.resize(sceneColliders.size());
    
    for (unsigned int x = 0; x < sceneColliders.size(); x++)
    {
        distance_matrix[x].resize(sceneColliders.size());
        for (unsigned int y = x; y < sceneColliders.size(); y++)
        {
            BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &sceneColliders.at(x)->bounding_box, &sceneColliders.at(y)->bounding_box);
            float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
            distance_matrix[x][y] = d;
            if (d < smallest_box && x!= y)
            {
                smallest_box = d;
                next_merge = {x,y};
            }
        }
    }

    //calculate distance matrix
    //combine the two closest and merge them
    for (unsigned int i = 0; i < sceneColliders.size() - 1; i++)
    {

        int array_pos_of_next_merge_obj_0 =  matrix_bb_tree_map[next_merge.x];
        int array_pos_of_next_merge_obj_1 =  matrix_bb_tree_map[next_merge.y];
        auto temp_bb = StructBoundingBox{};
        BoundingBoxHelper::get_combined_bounding_box(
            &temp_bb,
            &axis_aligned_bb_tree_[array_pos_of_next_merge_obj_0].bb,
            &axis_aligned_bb_tree_[array_pos_of_next_merge_obj_1].bb);

        //merge the smallest box
        auto temp = kdTreeElement{
            array_pos_of_next_merge_obj_0,
            array_pos_of_next_merge_obj_1,
            temp_bb
        };

        
        //TODO REMOVE THIS TEST
        auto c = new Cube3D(global_context_);
        scene_root_->addChild(c);
        c->setScale(BoundingBoxHelper::get_scale_of_bb(&temp_bb));
        c->color = {0, static_cast<float>(i)/static_cast<float>(sceneColliders.size()), 1};
        c->set_position_global(BoundingBoxHelper::get_center_of_bb(&temp_bb));
        ////////////////////TEST END //////////////////////////

        //add new bounding box containing both objects into the array
        axis_aligned_bb_tree_[sceneColliders.size()+i] = temp;

        int larger_position_in_matrix = std::max(next_merge.x,next_merge.y);
        int smaller_position_in_matrix = std::min(next_merge.x,next_merge.y);

        //remove the last element
        //row
        distance_matrix.erase(distance_matrix.begin()+larger_position_in_matrix);
        //colum
        for (int x = 0; x < distance_matrix.size(); x++)
        {
            distance_matrix[x].erase(distance_matrix[x].begin()+larger_position_in_matrix);
        }

        matrix_bb_tree_map.erase(matrix_bb_tree_map.begin()+larger_position_in_matrix);

        //replace the first element with the newly merged one 
        matrix_bb_tree_map.at(smaller_position_in_matrix)=sceneColliders.size()+i;
        //replace vertically 
        for (int x = 0; x < smaller_position_in_matrix; x++)
        {
            BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(x)].bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(smaller_position_in_matrix)].bb);
            float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
            distance_matrix[x][smaller_position_in_matrix] = d;
        }

        //replace horizontally 
        for (int x = smaller_position_in_matrix + 1; x < distance_matrix.size() - 1 - smaller_position_in_matrix; x++)
        {
  
            BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(smaller_position_in_matrix)].bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(x)].bb);
            float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
            distance_matrix[smaller_position_in_matrix][x] = d;
        }
        
        //get the smallest distance
        for (unsigned int x = 0; x < distance_matrix.size(); x++)
        {
            for (unsigned int y = x; y < distance_matrix.size(); y++)
            {
                BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(x)].bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(y)].bb);
                distance_matrix[x][y] = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
            }
        }
        
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Bounding Box stacking took: " << elapsed_seconds.count() << "s\n";
}

Object3D* SceneContext::get_root() const
{
    return scene_root_;
}

GlobalContext* SceneContext::get_global_context() const
{
    return global_context_;
}
