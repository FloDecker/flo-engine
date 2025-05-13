#include "Scene.h"

#include "Handle.h"
#include "../../Util/BoundingBoxHelper.h"
#include "Lighting/SkyBox/sky_box.h"
#include "../PhysicsEngine/PhysicsEngine.h"
#include "Modifiers/Implementations/Colliders/mesh_collider.h"
#include "SceneTools/gaussianizer.h"
#include "../Renderer/Texture/texture_buffer_object.h"

SceneRoot::SceneRoot(GlobalContext* global_context, Scene* scene): Object3D()
{
	global_context_ = global_context;
	this->scene_ = scene;
}

int SceneRoot::draw_entry_point(RenderContext* render_context) const
{
	for (auto& child : children)
	{
		child->draw_(render_context);
	}
	return 1;
}

Scene::Scene(GlobalContext* global_context, const std::string& name)
{
	global_context_ = global_context;
	physics_engine_ = new PhysicsEngine(this);
	//get ids of engine defined tags
	engine_light_point_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_LIGHT_POINT");
	engine_collider_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_COLLIDER");

	for (const auto c : all_collision_channels)
	{
		scene_bb[c] = new StackedBB(this);
	}

	scene_root_ = new SceneRoot(global_context, this);
	scene_root_->name = name;
	name_ = name;
	handle_ = new Handle(this);
	visual_debug_tools_ = new visual_debug_tools(this);
	light_pass_render_context_ = new RenderContext();
	light_pass_render_context_->light_pass_depth_only_shader = global_context->light_pass_depth_only_shader;
	light_pass_render_context_->pass = render_pass_lighting;


	surfels_texture_buffer_positions_ = new texture_buffer_object();
	surfels_texture_buffer_normals_ = new texture_buffer_object();
	surfels_texture_buffer_color_ = new texture_buffer_object();
	surfels_texture_buffer_radii_ = new texture_buffer_object();
	surfels_uniform_grid = new texture_buffer_object();
}


direct_light* Scene::get_scene_direct_light() const
{
	return direct_light_;
}

void Scene::register_collider(collider_modifier* collider)
{
	colliders_.push_back(collider);
	for (auto channel : collider->collision_channels())
	{
		scene_bb[channel]->insert_leaf_node(collider);
	}
}

void Scene::recalculate_collision_channel_bb_hierarchy(collision_channel channel) const
{
	scene_bb.at(channel)->recalculate();
}

std::vector<collider_modifier*> Scene::get_colliders(collision_channel collision_channel)
{
	std::vector<collider_modifier*> result;
	for (auto& collider : colliders_)
	{
		if (collider->collision_channels().contains(collision_channel))
		{
			result.push_back(collider);
		}
	}
	return result;
}

std::vector<collider_intersection> Scene::generate_overlaps_in_channel(collision_channel channel)
{
	const auto colliders = get_colliders(channel);

	const size_t num_colliders = colliders.size();

	std::vector<collider_intersection> result;
	for (size_t i = 0; i < num_colliders; ++i)
	{
		for (size_t j = i + 1; j < num_colliders; ++j)
		{
			// Start from i+1 to avoid duplicate checks
			const auto collider_a = colliders[i];
			const auto collider_b = colliders[j];

			const auto c = collider_b->check_intersection(collider_a);
			if (c.intersected)
			{
				result.emplace_back(collider_intersection(
						c,
						collider_a,
						collider_b)
				);
			}
		}
	}
	return result;
}

ray_cast_result Scene::ray_cast_in_scene_unoptimized(glm::vec3 origin, glm::vec3 direction, float max_distance,
                                                     collision_channel collision_channel)
{
	auto start = std::chrono::system_clock::now();

	ray_cast_result result;
	auto temp_result = ray_cast_result();
	double min_distance = std::numeric_limits<double>::max();
	for (auto collider : get_colliders(collision_channel))
	{
		auto bb = collider->get_world_space_bounding_box();
		if (BoundingBoxHelper::is_in_bounding_box(bb, origin) ||
			BoundingBoxHelper::ray_axis_aligned_bb_intersection(bb, origin, direction, &temp_result))
		{
			collider->ray_intersection_world_space(origin, direction, max_distance, true, &temp_result);
			if (temp_result.hit && temp_result.distance_from_origin < min_distance)
			{
				min_distance = temp_result.distance_from_origin;
				result = temp_result;
				result.object_3d = collider->get_parent();
			}
		}
	}
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;
	auto time_string = "ray cast in " + std::to_string(elapsed_seconds.count()) + "seconds\n";
	get_global_context()->logger->print_info(time_string);
	return result;
}

ray_cast_result Scene::ray_cast_in_scene(glm::vec3 origin, glm::vec3 direction, float max_distance,
                                         collision_channel collision_channel, Object3D* ignore)
{
	ray_cast_result result;
	result.distance_from_origin = std::numeric_limits<double>::max();
	auto bb_tree = scene_bb[collision_channel];
	bb_tree->scene_geometry_raycast(origin, direction, &result, max_distance, ignore);
	return result;
}

irradiance_information Scene::get_irradiance_information(glm::vec3 pos_ws, glm::vec3 normal_ws)
{
	//for (unsigned int i = 0; i < irradiance_samples; ++i)
	//{
	//	auto v = uniformHemisphereSample(normal_ws);
	//	auto r = ray_cast_in_scene(pos_ws, v, 4000, VISIBILITY);
	//	
	//}
	auto irradiance = irradiance_information();
	irradiance.pos = pos_ws;
	irradiance.normal = normal_ws;

	//auto r = ray_cast_in_scene(pos_ws + normal_ws * 0.1f, direct_light_->get_light_direction(), 4000, VISIBILITY);
	//if (r.hit)
	//{
	//	irradiance.color = glm::vec3(0.0, 0.0, 0.0);
	//}
	//else
	//{
	//	irradiance.color = direct_light_->color;
	//}

	float ambient_hits = 0;
	for (int i = 0; i < gi_primary_rays; i++)
	{
		auto d = uniformHemisphereSample(normal_ws);
		auto r = ray_cast_in_scene(pos_ws + normal_ws * 0.1f, d, 4000, VISIBILITY);
		ambient_hits+= (r.hit) ? 0.0 : 1.0/static_cast<float>(gi_primary_rays);
	}
	irradiance.color+=ambient_hits * glm::vec3(1.0, 1.0, 1.0);
	return irradiance;
}

glm::vec3 Scene::uniformHemisphereSample(glm::vec3 normal)
{
	float u1 = static_cast<float>(rand()) / RAND_MAX; // Random value in [0,1]
	float u2 = static_cast<float>(rand()) / RAND_MAX;

	float theta = acos(1 - u1);
	float phi = 2.0f * glm::pi<float>() * u2;

	glm::vec3 vector_local = {
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	};

	float Tx, Ty, Tz;
	if (fabs(normal.x) > fabs(normal.z))
	{
		// Choose a tangent that is not parallel to N
		Tx = -normal.y;
		Ty = normal.x;
		Tz = 0;
	}
	else
	{
		Tx = 0;
		Ty = -normal.z;
		Tz = normal.y;
	}

	// Normalize tangent
	float lenT = sqrt(Tx * Tx + Ty * Ty + Tz * Tz);
	Tx /= lenT;
	Ty /= lenT;
	Tz /= lenT;

	// Compute bitangent B = N x T
	float Bx = normal.y * Tz - normal.z * Ty;
	float By = normal.z * Tx - normal.x * Tz;
	float Bz = normal.x * Ty - normal.y * Tx;

	// Transform sampled vector to world space
	return {
		vector_local.x * Tx + vector_local.y * Bx + vector_local.z * normal.x,
		vector_local.x * Ty + vector_local.y * By + vector_local.z * normal.y,
		vector_local.x * Tz + vector_local.y * Bz + vector_local.z * normal.z
	};
}

void Scene::init_surfels_buffer()
{
	if (has_surfels_buffer_) {
		global_context_->logger->print_warning("Scene already initialized surfel buffers");
		return;
	};

	global_context_->logger->print_info("Initializing surfel buffer...");
	global_context_->logger->print_info(std::format("Surfel buffer size : {}", SURFEL_BUFFER_AMOUNT));

	
	surfels_uniform_grid->init_u_int_2(SURFELS_GRID_SIZE*SURFELS_GRID_SIZE*SURFELS_GRID_SIZE);
	surfels_texture_buffer_positions_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_normals_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_color_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_radii_->init_float(SURFEL_BUFFER_AMOUNT);
	global_context_->logger->print_info(std::format("Surfel buffer total memory : {}",
		sizeof(glm::vec3) * SURFEL_BUFFER_AMOUNT * 3 + sizeof(float) * SURFEL_BUFFER_AMOUNT)
		);
	global_context_->logger->print_info(std::format("Surfel grid total memory : {}",
	sizeof(unsigned int) * 2 * SURFELS_GRID_SIZE*SURFELS_GRID_SIZE*SURFELS_GRID_SIZE)
	);
	
	has_surfels_buffer_ = true;
	
}

void Scene::recalculate_surfels()
{

	if (!has_surfels_buffer_)
	{
		init_surfels_buffer();
	}
	
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<float> radii;

	unsigned int size = 0;
	for (auto g : gaussianinzers_)
	{
		for (auto s : g->samples())
		{
			//positions.push_back(s.mean);
			positions.push_back(s.mean);
			normals.push_back(s.normal);
			radii.push_back(s.radius);

			auto i = get_irradiance_information(s.mean, s.normal);
			colors.push_back(i.color);
			size++;
		}
	}

	std::map<unsigned int ,std::vector<unsigned int>> buckets;
	
	
	//sort surfels into corresponding bucket
	for (unsigned int i = 0; i < size; i++)
	{
		auto p = positions[i];
		auto n = normals[i];
		unsigned int buckets_for_surfle[8] = {};
		unsigned int buckets_length = get_surfel_buckets_from_ws_pos(p,n, buckets_for_surfle);
		//unsigned int bucket = get_surfel_bucket_from_ws_pos(p);

		for (int x = 0; x < buckets_length; x++)
		{
			auto bucket = buckets_for_surfle[x];
			if (buckets.contains(bucket))
			{
				buckets[bucket].push_back(i);
			} else
			{
				buckets[bucket] = std::vector<unsigned int>{i};
			}
		}

	}

	std::vector<glm::vec3> positions_surfel_stack;
	std::vector<glm::vec3> normals_surfel_stack;
	std::vector<glm::vec3> colors_surfel_stack;
	std::vector<float> radii_surfel_stack;


	
	unsigned int surfel_stack_counter = 0;
	for (auto bucket : buckets)
	{
		auto bucket_pos_in_grid = bucket.first;
		std::vector<unsigned int> bucket_surfel_ids = bucket.second;
		auto data = std::vector<glm::u32vec2>{glm::u32vec2(surfel_stack_counter,static_cast<unsigned>(bucket_surfel_ids.size()))};
		surfels_uniform_grid->update_u_int_2(&data, bucket_pos_in_grid);
		for (auto surfel_ids : bucket.second)
		{
			positions_surfel_stack.push_back(positions.at(surfel_ids));
			normals_surfel_stack.push_back(normals.at(surfel_ids));
			colors_surfel_stack.push_back(colors.at(surfel_ids));
			radii_surfel_stack.push_back(radii.at(surfel_ids));
			surfel_stack_counter++;
		}
	}
	
	bool success = true;
	success&= surfels_texture_buffer_positions_->update_vec3(&positions_surfel_stack, 0);
	success&= surfels_texture_buffer_normals_->update_vec3(&normals_surfel_stack, 0);
	success&= surfels_texture_buffer_color_->update_vec3(&colors_surfel_stack, 0);
	success&= surfels_texture_buffer_radii_->update_float(&radii_surfel_stack, 0);
	if (success)
	{
		global_context_->logger->print_info("surfels updated");

	} else
	{
		global_context_->logger->print_warning("surfels couldn't been updated correctly");
	}

}

//TODO : currently the surfle is considered a circle and is therefor placed in buckets it doesent belong to,
//this doesent affect visuals but performance 
unsigned int Scene::get_surfel_buckets_from_ws_pos(glm::vec3 ws_pos, glm::vec3 ws_normal, unsigned int buckets[8])
{
	buckets[0] = get_surfel_bucket_from_ws_pos(ws_pos);
	auto surfel_bucket_center = get_surfel_bucket_center(ws_pos);
	auto center_pos_v = ws_pos - surfel_bucket_center;
	center_pos_v.x = center_pos_v.x > 0 ? 1.0f : -1.0f;
	center_pos_v.y = center_pos_v.y > 0 ? 1.0f : -1.0f;
	center_pos_v.z = center_pos_v.z > 0 ? 1.0f : -1.0f;
	center_pos_v = normalize(center_pos_v);

	StructBoundingSphere b = {ws_pos, SURFELS_BUCKET_SIZE * 0.5f};
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
	unsigned int filled_buckets = 1;
	
	auto half_size = glm::vec3(0.5) * SURFELS_BUCKET_SIZE;
	for (int i = 0 ; i < 7; i++)
	{
		auto center_neighbour = surfel_bucket_center + glm::normalize(center_pos_v * component_multiplier[i]) * SURFELS_BUCKET_SIZE;
		bb.min = center_neighbour - half_size;
		bb.max = center_neighbour + half_size;
		if (BoundingBoxHelper::are_overlapping(&bb, &b))
		{
			buckets[filled_buckets] = get_surfel_bucket_from_ws_pos(center_neighbour);
			filled_buckets++;
			
		}
	}
	return filled_buckets;
}


glm::vec3 Scene::get_surfel_bucket_center(glm::vec3 ws_pos) const
{
	ws_pos /= SURFELS_BUCKET_SIZE;
	ws_pos = glm::floor(ws_pos) + 0.5f ;
	return  ws_pos * SURFELS_BUCKET_SIZE;
}

unsigned int Scene::get_surfel_bucket_from_ws_pos(glm::vec3 ws_pos) const
{
	ws_pos /= SURFELS_BUCKET_SIZE;
	ws_pos += SURFELS_GRID_SIZE*0.5f;
	ws_pos = glm::floor(ws_pos);
	return ws_pos.x + SURFELS_GRID_SIZE * ws_pos.y + SURFELS_GRID_SIZE * (SURFELS_GRID_SIZE * ws_pos.z);
}


void Scene::recalculate_at(Object3D* parent)
{
	//sort objets by their tag
	if (parent->has_tag(engine_light_point_id_))
	{
		this->scenePointLights.insert(dynamic_cast<PointLight*>(parent));
	}

	const auto children = parent->get_children();
	for (Object3D* child : children)
	{
		this->recalculate_at(child);
	}
}

void Scene::recalculate_from_root()
{
	recalculate_at(scene_root_);
	for (const auto c : all_collision_channels)
	{
		recalculate_collision_channel_bb_hierarchy(c);
	}
}


void Scene::deselect()
{
	if (!_has_selected_object) { return; }
	_has_selected_object = false;
	this->selected_object = nullptr;
	handle_->detach();
}

void Scene::select_object(Object3D* object)
{
	handle_->attach_to_object(object);
	_has_selected_object = true;
	this->selected_object = object;
}

Handle* Scene::handle() const
{
	return handle_;
}

Object3D* Scene::get_selected_object() const
{
	return selected_object;
}

bool Scene::has_selected_object() const
{
	return _has_selected_object;
}

StructColorRange* Scene::get_ao_color_at(int samples, glm::vec3 ws_pos) const
{
	return sky_box_->get_sky_box_ao_color_range();
}

void Scene::get_gaussian_approx_at(glm::vec3 ws_pos, std::vector<gaussian>* result) const
{
	if (gaussianinzers_.size() == 0)
	{
		return;
	}

	for (int i = 0; i < gi_primary_rays; ++i)
	{
		if (gaussianinzers_.at(0)->samples().size() > i)
		{
			result->push_back(gaussianinzers_.at(0)->samples().at(i));
		}
	}
}

void Scene::register_global_light(direct_light* direct_light)
{
	if (direct_light_ != nullptr)
	{
		std::cerr << "Scene already has a direct light\n";
		return;
	}
	has_direct_light_ = true;
	direct_light_ = direct_light;
}

void Scene::register_sky_box(sky_box* skybox)
{
	if (direct_light_ != nullptr)
	{
		std::cerr << "Scene already has a skybox light\n";
		return;
	}
	has_sky_box = true;
	sky_box_ = skybox;
}

void Scene::register_gaussianizer(gaussianizer* gaussianizer)
{
	gaussianinzers_.push_back(gaussianizer);
}

Object3D* Scene::get_root() const
{
	return scene_root_;
}

GlobalContext* Scene::get_global_context() const
{
	return global_context_;
}

PhysicsEngine* Scene::get_physics_engine() const
{
	return physics_engine_;
}

StackedBB* Scene::get_bb(collision_channel c) const
{
	return scene_bb.at(c);
}

visual_debug_tools* Scene::get_debug_tools() const
{
	return visual_debug_tools_;
}

void Scene::draw_scene(RenderContext* render_context) const
{
	scene_root_->draw_entry_point(render_context);
}

void Scene::draw_debug_tools(const RenderContext* render_context) const
{
	visual_debug_tools_->draw_debug_tools(render_context->deltaTime);
}

void Scene::light_pass(camera* current_camera) const
{
	//direct light
	if (has_direct_light_)
	{
		direct_light_->set_light_center_position(*current_camera->getWorldPosition());
		direct_light_->render_to_light_map();
		light_pass_render_context_->light = direct_light_;
		light_pass_render_context_->camera = current_camera;
		scene_root_->draw_entry_point(light_pass_render_context_);
		direct_light_->light_map()->generate_mip_map();
	}
}

void Scene::get_colliders_in_bounding_box(StructBoundingBox* bounding_box, collision_channel channel,
                                          std::vector<collider_modifier*>* result)
{
	for (auto element : get_colliders(channel))
	{
		if (BoundingBoxHelper::are_overlapping(element->get_world_space_bounding_box(), bounding_box))
		{
			result->push_back(element);
		}
	}
}


std::vector<glm::vec3>* Scene::get_polygons_in_bounding_box(StructBoundingBox* bounding_box) const
{
	std::cout << "Scene::get_polygons_in_bounding_box is not implemented \n" << std::endl;
	return nullptr;
	// auto out = new std::vector<glm::vec3>;
	// std::vector<Collider*> colliders_in_bb = *get_colliders_in_bounding_box(bounding_box, TODO);
	// for (Collider* c : colliders_in_bb)
	// {
	// 	if (c->get_collider_type() == 1) // collider is a mesh collider
	// 	{
	// 		auto c_m = dynamic_cast<MeshCollider*>(c);
	// 		for (auto vertex_array : *c_m->get_vertex_arrays()) //get vertices of mesh collider
	// 		{
	// 			for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3) //check every polygon if in bb
	// 			{
	// 				glm::vec3 v_0_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i)).position);
	// 				glm::vec3 v_1_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i + 1)).position);
	// 				glm::vec3 v_2_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i + 2)).position);
	//
	// 				if (BoundingBoxHelper::intersects_polygon(bounding_box, v_0_ws, v_1_ws, v_2_ws))
	// 				{
	// 					out->push_back(v_0_ws);
	// 					out->push_back(v_1_ws);
	// 					out->push_back(v_2_ws);
	// 				}
	//
	// 				//check if this vertex is in bounding box
	// 			}
	// 		}
	// 	}
	// }
	//
	// return out;
}

std::vector<std::tuple<mesh_collider*, std::vector<vertex_array_filter>*>>* Scene::get_triangles_in_bounding_box(
	StructBoundingBox* bounding_box) const
{
	std::cout << "Scene::get_triangles_in_bounding_box is not implemented \n" << std::endl;
	return nullptr;
	//
	// auto out = new std::vector<std::tuple<MeshCollider*, std::vector<vertex_array_filter>*>>;
	// std::vector<Collider*> colliders_in_bb = *get_colliders_in_bounding_box(bounding_box, TODO);
	// for (Collider* c : colliders_in_bb)
	// {
	// 	if (c->get_collider_type() == 1) // collider is a mesh collider
	// 	{
	// 		auto c_m = dynamic_cast<MeshCollider*>(c);
	// 		auto v_f_array = new std::vector<vertex_array_filter>;
	// 		for (int i = 0; i < c_m->get_vertex_arrays()->size(); i++) //get vertices of mesh collider
	// 		{
	// 			auto vertex_array = c_m->get_vertex_arrays()->at(i);
	// 			auto v_f = new vertex_array_filter;
	// 			for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3) //check every polygon if in bb
	// 			{
	// 				glm::vec3 v_0_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i)).position);
	// 				glm::vec3 v_1_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i + 1)).position);
	// 				glm::vec3 v_2_ws = c_m->transform_vertex_to_world_space(
	// 					vertex_array->vertices->at(vertex_array->indices->at(i + 2)).position);
	//
	// 				if (BoundingBoxHelper::intersects_polygon(bounding_box, v_0_ws, v_1_ws, v_2_ws))
	// 				{
	// 					v_f->indices.push_back(i);
	// 				}
	//
	// 				//check if this vertex is in bounding box
	// 			}
	// 			if (!v_f->indices.empty())
	// 			{
	// 				v_f->vertex_array_id = i;
	// 				v_f_array->emplace_back(*v_f);
	// 			}
	// 		}
	// 		if (!v_f_array->empty())
	// 		{
	// 			auto temp = std::make_tuple(c_m, v_f_array);
	// 			out->emplace_back(temp);
	// 		}
	// 	}
	// }
	//
	// return out;
}
