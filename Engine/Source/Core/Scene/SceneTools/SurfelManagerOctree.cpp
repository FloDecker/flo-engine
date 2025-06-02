#include "SurfelManagerOctree.h"

#include <random>

#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"
#include "../../Renderer/Texture/texture_buffer_object.h"

SurfelManagerOctree::SurfelManagerOctree(Scene* scene)
{
    scene_ = scene;
    surfels_texture_buffer_positions_ = new texture_buffer_object();
    surfels_texture_buffer_normals_ = new texture_buffer_object();
    surfels_texture_buffer_color_ = new texture_buffer_object();
    surfels_texture_buffer_radii_ = new texture_buffer_object();
    surfels_octree = new texture_buffer_object();

}

void SurfelManagerOctree::clear_samples()
{
    surfels_.clear();
}

void SurfelManagerOctree::draw_ui()
{
    if (ImGui::Button("Generate surfels"))
    {
        recalculate_surfels();
    }
    ImGui::Checkbox("Draw debug tools", &draw_debug_tools_);

    if (draw_debug_tools_)
    {
        for (auto sample : surfels_)
        {
            scene_->get_debug_tools()->draw_debug_point(sample.mean);
        }
    }

    ImGui::DragFloat("points per square meter", &points_per_square_meter);
    if (ImGui::Button("Recalculate"))
    {
        snap_samples_to_closest_surface();
    }
    ImGui::DragInt("surfel primary rays", &gi_primary_rays);
    ImGui::DragInt("surfel updates per tick", &update_surfels_per_tick);
}


int SurfelManagerOctree::get_octree_level_for_surfel(const surfel* surfel)
{
    auto r = surfel->radius;
    int level_surfel = ceil(log2(r));
    int level_bounds = ceil(log2(total_extension));
    return std::min(level_bounds - level_surfel, octree_levels);
}

static uint32_t combine(const uint32_t a, const uint32_t b, const uint32_t mask)
{
    return (a & ~mask) | (b & mask);
}

static uint8_t get_pos_of_next_surfel_index_(glm::vec3 pos_relative)
{
    uint8_t r = 0b00000000;
    if (pos_relative.x >= 0)
    {
        r |= (1 << 2);
    }

    if (pos_relative.y >= 0)
    {
        r |= (1 << 1);
    }

    if (pos_relative.z >= 0)
    {
        r |= (1 << 0);
    }
    return r;
}

bool SurfelManagerOctree::insert_surfel_into_octree(const surfel* surfel)
{
    auto target_level = get_octree_level_for_surfel(surfel);
    return insert_surfel_into_octree_recursive(surfel, 0, {0, 0, 0}, target_level, 0, true, surfel->mean);
}

bool SurfelManagerOctree::insert_surfel_in_surrounding_buckets(const surfel* surfel,
                                                               int target_layer)
{
    auto surfel_bucket_center = get_surfel_bucket_center(surfel->mean, target_layer);

    auto center_pos_v = surfel->mean - surfel_bucket_center;
    center_pos_v.x = center_pos_v.x > 0 ? 1.0f : -1.0f;
    center_pos_v.y = center_pos_v.y > 0 ? 1.0f : -1.0f;
    center_pos_v.z = center_pos_v.z > 0 ? 1.0f : -1.0f;
    center_pos_v = normalize(center_pos_v);

    float bucket_size = total_extension/pow(2,target_layer);
    
    
    StructBoundingSphere b = {surfel->mean, bucket_size * 0.5f};
    StructBoundingBox bb = {};
    const glm::vec3 component_multiplier[8] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
    };
    
    auto half_size = glm::vec3(0.5) * bucket_size;
    for (int i = 0 ; i < 7; i++)
    {
        auto center_neighbour = surfel_bucket_center + glm::normalize(center_pos_v * component_multiplier[i]) * bucket_size;
        bb.min = center_neighbour - half_size;
        bb.max = center_neighbour + half_size;
        if (BoundingBoxHelper::are_overlapping(&bb, &b))
        {
            insert_surfel_into_octree_recursive(surfel,0,{0,0,0},target_layer,0,false, center_neighbour);
        }
    }
    
    return true;
}


glm::vec3 SurfelManagerOctree::get_surfel_bucket_center(glm::vec3 ws_pos, int level) const
{
    if (level == 0) return glm::vec3(0, 0, 0);
    float bucket_amount_at_level = pow(2,level);
    auto bucket_extension_ws = total_extension / bucket_amount_at_level;
    ws_pos /= total_extension*0.5;
    ws_pos *= bucket_amount_at_level*0.5f;
    ws_pos = glm::floor(ws_pos) + 0.5f;
    return  ws_pos * bucket_extension_ws;
}



bool SurfelManagerOctree::insert_surfel_into_octree_recursive(const surfel* surfel_to_insert, int current_layer,
                                                              glm::vec3 current_center, int target_layer,
                                                              int current_octree_element_index, bool insert_into_surroundings, glm::vec3 target_pos)
{
    auto current_octree_element = &octree_[current_octree_element_index];
    if (target_layer < current_layer)
    {
        return false;
    }
    if (target_layer == current_layer)
    {
        //insert -> write this to gpu memory
        uint32_t surfel_bucket_pointer;
        auto surfel_count = get_surfel_count_in_octree_element(current_octree_element);
        if (surfel_count <= 0)
        {
            //create new
            create_space_for_new_surfel_data(surfel_bucket_pointer);
            current_octree_element->surfels_at_layer_pointer = surfel_bucket_pointer;
        } else if (surfel_count > SURFEL_BUCKET_SIZE_ON_GPU)
        {
            //TODO resize surfel buckets when more space is needed  
            memory_limitation_bucket_size++;
            return false;
        }
        auto pos = current_octree_element->surfels_at_layer_pointer + surfel_count;
        surfel_to_insert->surfel_in_gpu_memory->push_back(pos);
        if (insert_into_surroundings)
        {
            insert_surfel_in_surrounding_buckets(surfel_to_insert, target_layer);
        }
        if (upload_surfel_data(surfel_to_insert))
        {
            increment_surfel_count_in_octree_element(current_octree_element);
            return true; //data is successfully uploaded to gpu memeory 
        }
        return false; //data couldn't be uploaded
    }

    auto pos_relative = target_pos - current_center;
    auto next_index = get_pos_of_next_surfel_index_(pos_relative);
    uint32_t next_octree_index;
    if (is_child_octree_bit_set_at_(current_octree_element, next_index))
    {
        //child already exists
        next_octree_index = current_octree_element->next_layer_surfels_pointer[next_index];
    }
    else
    {
        uint32_t new_index;
        if (create_new_octree_element(new_index))
        {
            //could create new index
            next_octree_index = new_index;

            //set the bit to true at the index and insert the next pointer at the corresponding position
            current_octree_element->next_layer_surfels_pointer[next_index] = next_octree_index;
            set_child_octree_bit_at_(current_octree_element, next_index);
        }
        else
        {
            //alrady full
            return false;
        }
    }
    glm::vec3 next_center = current_center + glm::vec3(
        pos_relative.x >= 0 ? 1.0 : -1.0,
        pos_relative.y >= 0 ? 1.0 : -1.0,
        pos_relative.z >= 0 ? 1.0 : -1.0
    ) * 0.5f * (total_extension / powf(2.0f, current_layer + 1));


    return insert_surfel_into_octree_recursive(surfel_to_insert, ++current_layer, next_center, target_layer,
                                               next_octree_index, insert_into_surroundings, target_pos);
}

bool SurfelManagerOctree::create_new_octree_element(uint32_t& index)
{
    if (next_free_spot_in_octree_ >= SURFEL_OCTREE_SIZE)
    {
        memory_limitation_octree_size++;
        return false;
    }

    octree_[next_free_spot_in_octree_] = {};
    next_free_spot_in_octree_++;
    index = next_free_spot_in_octree_ - 1;
    return true;
}

bool SurfelManagerOctree::create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array)
{
    if (SURFELS_BOTTOM_LEVEL_SIZE  <= surfel_stack_pointer / SURFEL_BUCKET_SIZE_ON_GPU)
    {
        //the surfel buffer on the gpu is already full
        memory_limitation_count_bottom_size++;
        return false;
    }

    pointer_ins_surfel_array = surfel_stack_pointer;
    surfel_stack_pointer += SURFEL_BUCKET_SIZE_ON_GPU;
    return true;
}

//TODO: veryyyy inefficient
bool SurfelManagerOctree::upload_surfel_data(const surfel* surfel_to_insert) const
{
    bool s = true;
    for (unsigned int insert_at_index : *surfel_to_insert->surfel_in_gpu_memory)
    {
        s &= surfels_texture_buffer_positions_->update_vec3_single(&surfel_to_insert->mean, insert_at_index);
        s &= surfels_texture_buffer_normals_->update_vec3_single(&surfel_to_insert->normal, insert_at_index);
        s &= surfels_texture_buffer_color_->update_vec3_single(&surfel_to_insert->diffuse_irradiance, insert_at_index);
        s &= surfels_texture_buffer_radii_->update_float_single(&surfel_to_insert->radius, insert_at_index);
    }
    return s;
}

void SurfelManagerOctree::dump_surfle_octree_to_gpu_memory_()
{
    surfels_octree->update_u_int(&octree_[0].surfels_at_layer_amount, next_free_spot_in_octree_ * 10, 0);
}

void SurfelManagerOctree::snap_samples_to_closest_surface()
{
    clear_samples();
    //StructBoundingBox boundingbox = {lower_left, upper_right};
    //auto colliders_in_bb = std::vector<collider_modifier*>();
    auto colliders = scene_->get_colliders(VISIBILITY);

    auto points = std::vector<vertex>();
    for (collider_modifier* collider : colliders)
    {
        collider->scatter_points_on_surface(&points, points_per_square_meter);
    }

    for (auto point : points)
    {
        //TODO :remove me thats a test
        //std::random_device rd; // Seed the random number generator
        //std::mt19937 gen(rd()); // Mersenne Twister PRNG
        //std::uniform_real_distribution<float> float_dist_0_1(1.0f / points_per_square_meter, 5.0f); 
        surfels_.push_back({point.position, point.normal, {1, 1, 1}, 1.0f / points_per_square_meter});
    }
}

std::vector<surfel> SurfelManagerOctree::samples()
{
    return surfels_;
}

void SurfelManagerOctree::add_surfel_uniforms_to_shader(ShaderProgram* shader) const
{
    shader->addTexture(surfels_texture_buffer_color_, "surfels_texture_buffer_color_");
    shader->addTexture(surfels_texture_buffer_normals_, "surfels_texture_buffer_normals_");
    shader->addTexture(surfels_texture_buffer_positions_, "surfels_texture_buffer_positions_");
    shader->addTexture(surfels_texture_buffer_radii_, "surfels_texture_buffer_radii_");
    shader->addTexture(surfels_octree, "surfels_uniform_grid");
}


void SurfelManagerOctree::init_surfels_buffer()
{
    if (has_surfels_buffer_)
    {
        scene_->get_global_context()->logger->print_warning("Scene already initialized surfel buffers");
        return;
    }

    scene_->get_global_context()->logger->print_info("Initializing surfel buffer...");
    scene_->get_global_context()->logger->print_info(std::format("Surfel buffer size : {}", SURFEL_OCTREE_SIZE));

    surfels_octree->init_u_int(SURFEL_OCTREE_SIZE * 10);


    surfels_texture_buffer_positions_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
    surfels_texture_buffer_normals_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
    surfels_texture_buffer_color_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
    surfels_texture_buffer_radii_->init_float(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);

    scene_->get_global_context()->logger->print_info(std::format("Surfel buffer total memory : {}",
                                                                 sizeof(glm::vec3) * SURFEL_OCTREE_SIZE * 3 + sizeof(
                                                                     float) * SURFEL_OCTREE_SIZE)
    );
    scene_->get_global_context()->logger->print_info(std::format("Surfel grid total memory : {}",
                                                                 sizeof(unsigned int) * 2 * SURFELS_BOTTOM_LEVEL_SIZE *
                                                                 SURFELS_BOTTOM_LEVEL_SIZE * SURFELS_BOTTOM_LEVEL_SIZE)
    );

    has_surfels_buffer_ = true;
}

void SurfelManagerOctree::reset_surfels_buffer()
{
    next_free_spot_in_octree_ = 1;
    last_updated_surfel = 0;
    surfel_stack_pointer = 0;
    free(octree_);
    octree_ = new surfel_octree_element[1000000];
    octree_[0] = {.surfels_at_layer_amount = 0, .surfels_at_layer_pointer = 0, .next_layer_surfels_pointer = {}};
}

void SurfelManagerOctree::recalculate_surfels()
{
    if (!has_surfels_buffer_)
    {
        init_surfels_buffer();
    }
    reset_surfels_buffer();

    
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors;
    std::vector<float> radii;

    memory_limitation_octree_size=0;
    memory_limitation_count_bottom_size=0;
    memory_limitation_bucket_size=0;

    int sucessfull_inserts = 0;
    for (auto s : surfels_)
    {
        //positions.push_back(s.mean);
        positions.push_back(s.mean);
        normals.push_back(s.normal);
        radii.push_back(s.radius);

        auto i = scene_->get_irradiance_information(s.mean, s.normal, gi_primary_rays);
        colors.push_back(i.color);
        sucessfull_inserts += insert_surfel_into_octree(&s) ? 1 : 0;
    }

    dump_surfle_octree_to_gpu_memory_();
    scene_->get_global_context()->logger->print_info(std::format(
        "surfel octree created {} surfels out of {} have been inserted ",
        sucessfull_inserts, surfels_.size()));

    scene_->get_global_context()->logger->print_info(std::format("Memory limitation for octree size was met in {} cases", memory_limitation_octree_size));
    scene_->get_global_context()->logger->print_info(std::format("Memory limitation for bottom level buckets was met in {} cases", memory_limitation_count_bottom_size));
    scene_->get_global_context()->logger->print_info(std::format("Memory limitation for single bucket size was met in {} cases", memory_limitation_bucket_size));
}

void SurfelManagerOctree::update_surfels()
{
    if (surfels_.size() == 0) return;
    for (int i = 0; i < update_surfels_per_tick; i++)
    {
        surfel *s = &surfels_.at(last_updated_surfel);
        auto irradiance_info = scene_->get_irradiance_information(s->mean, s->normal, gi_primary_rays);
        s->diffuse_irradiance = s->diffuse_irradiance * static_cast<float>(s->samples) + irradiance_info.color * static_cast<float>(gi_primary_rays);
        s->samples+=gi_primary_rays;
        s->diffuse_irradiance /= static_cast<float>(s->samples);
        upload_surfel_data(s);
        last_updated_surfel++;
        if (last_updated_surfel >= surfels_.size())
        {
            last_updated_surfel = 0;
        }
        
    }
}


bool SurfelManagerOctree::is_child_octree_bit_set_at_(const surfel_octree_element* surfel_octree_element,
                                                      const uint8_t pos)
{
    return (surfel_octree_element->surfels_at_layer_amount & (1 << (31 - pos))) != 0;
}

void SurfelManagerOctree::set_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, const uint8_t pos)
{
    surfel_octree_element->surfels_at_layer_amount |= (1 << (31 - pos));
}

//TODO: test this with different radii 
void SurfelManagerOctree::increment_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element)
{
    constexpr uint32_t mask = 0xFF000000;
    uint32_t amount = get_surfel_count_in_octree_element(surfel_octree_element);
    amount++;
    surfel_octree_element->surfels_at_layer_amount = (surfel_octree_element->surfels_at_layer_amount & mask) | amount;
}

unsigned int SurfelManagerOctree::get_surfel_count_in_octree_element(const surfel_octree_element* surfel_octree_element)
{
    constexpr uint32_t mask = 0x00FFFFFF;
    return surfel_octree_element->surfels_at_layer_amount & mask;
}

