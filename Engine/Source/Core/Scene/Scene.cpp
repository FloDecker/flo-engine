#include "Scene.h"
#include <utility>

#include "Handle.h"
#include "../Scene/Collider.h"
#include "Lighting/SkyBox/sky_box.h"

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
	//get ids of engine defined tags
	engine_light_point_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_LIGHT_POINT");
	engine_collider_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_COLLIDER");
	scene_bb = new StackedBB(&sceneColliders);
	scene_root_ = new SceneRoot(global_context, this);
	scene_root_->name = name;
	recalculate_from_root();
	name_ = name;
	handle_ = new Handle(this);
	visual_debug_tools_ = new visual_debug_tools(this);
	
	light_pass_render_context_ = new RenderContext();
	light_pass_render_context_->pass = render_pass_lighting;
}


direct_light *Scene::get_scene_direct_light() const
{
	return direct_light_;
}


void Scene::recalculate_at(Object3D* parent)
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

void Scene::recalculate_from_root()
{
	recalculate_at(scene_root_);
	calculateColliderBoundingBoxes();
	scene_bb->recalculate();
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


std::pair<bool, Object3D*> Scene::pixel_picker(glm::vec3 view_pos_world_space, glm::vec3 direction) const
{
	auto pixel_picker_framebuffer_object = framebuffer_object();
	auto pixel_picker_result = Texture2D();
	auto pixel_picker_result_depth = Texture2D();
	
	pixel_picker_result.initialize_as_pixel_picker();
	pixel_picker_result_depth.initialize_as_depth_map_render_target(10,10);
	pixel_picker_framebuffer_object.attach_texture_as_color_buffer(&pixel_picker_result);
	pixel_picker_framebuffer_object.attach_texture_as_depth_buffer(&pixel_picker_result_depth);

	
	ShaderProgram p = ShaderProgram();
	p.loadFromFile("EngineContent/Shader/PixelPickerPass.glsl");
	p.set_shader_header_include(DEFAULT_HEADERS,false);
	p.set_shader_header_include(PIXEL_PICKING,true);
	p.compileShader();
	RenderContext r = RenderContext();
	r.camera = new camera(1,1);
	r.pass = render_pass_custom;
	r.custom_shader = &p;
	
	r.camera->calculate_view(view_pos_world_space, direction);
	glDisable(GL_BLEND);
	int idpre = pixel_picker_framebuffer_object.read_pixel_as_integer(0,0);

	pixel_picker_framebuffer_object.render_to_framebuffer();
	custom_pass(&r);
	glEnable(GL_BLEND);
	int id = pixel_picker_framebuffer_object.read_pixel_as_integer(0,0);
	global_context_->logger->print_info(std::to_string(idpre));
	global_context_->logger->print_info(std::to_string(id));
	return std::pair<bool, Object3D*>(true, nullptr);
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

unsigned int Scene::register_object(Object3D* object)
{
	const auto id = get_new_id();
	objects_[id] = object;
	return id;
}

std::pair<bool,Object3D*> Scene::get_object_by_id(unsigned int id) const
{
	if (objects_.contains(id)) {
		return std::make_pair(true, objects_.at(id));
	} 
	return std::make_pair(false, nullptr);
}


unsigned int Scene::get_new_id()
{
	running_object_id_++;
	return running_object_id_;
}


void Scene::register_global_light(direct_light* direct_light)
{
	if (direct_light_ != nullptr)
	{
		std::cerr<<"Scene already has a direct light\n";
		return;
	}
	has_direct_light_ = true;
	direct_light_ = direct_light;
}

void Scene::register_sky_box(sky_box* skybox)
{
	if (direct_light_ != nullptr)
	{
		std::cerr<<"Scene already has a skybox light\n";
		return;
	}
	has_sky_box = true;
	sky_box_ = skybox;
}


void Scene::calculateColliderBoundingBoxes()
{
	for (auto collider : sceneColliders)
	{
		collider->calculate_world_space_bounding_box();

		//TODO REMOVE THIS TEST
		//auto c = new Cube3D(global_context_);
		//scene_root_->addChild(c);
		//c->setScale(BoundingBoxHelper::get_scale_of_bb(&collider->bounding_box));
		//c->color = {0, 0, 1};
		//c->set_position_global(BoundingBoxHelper::get_center_of_bb(&collider->bounding_box));
		////////////////////TEST END //////////////////////////
	}
}

Object3D* Scene::get_root() const
{
	return scene_root_;
}

GlobalContext* Scene::get_global_context() const
{
	return global_context_;
}

StackedBB* Scene::get_bb() const
{
	return scene_bb;
}

visual_debug_tools* Scene::get_debug_tools() const
{
	return visual_debug_tools_;
}

void Scene::draw_scene(RenderContext* render_context) const
{
	scene_root_->draw_entry_point(render_context);
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

void Scene::custom_pass(RenderContext* render_context) const
{
	scene_root_->draw_entry_point(render_context);
}

std::vector<Collider*>* Scene::get_colliders_in_bounding_box(StructBoundingBox* bounding_box) const
{
	auto out = new std::vector<Collider*>;
	for (auto element : sceneColliders)
	{
		if (BoundingBoxHelper::are_overlapping(&element->bounding_box, bounding_box))
		{
			out->push_back(element);
		}
	}
	return out;
}


std::vector<glm::vec3>* Scene::get_polygons_in_bounding_box(StructBoundingBox* bounding_box) const
{
	auto out = new std::vector<glm::vec3>;
	std::vector<Collider*> colliders_in_bb = *get_colliders_in_bounding_box(bounding_box);
	for (Collider* c : colliders_in_bb)
	{
		if (c->get_collider_type() == 1) // collider is a mesh collider
		{
			auto c_m = dynamic_cast<MeshCollider*>(c);
			for (auto vertex_array : *c_m->get_vertex_arrays()) //get vertices of mesh collider
			{
				for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3) //check every polygon if in bb
				{
					glm::vec3 v_0_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i)).position);
					glm::vec3 v_1_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i + 1)).position);
					glm::vec3 v_2_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i + 2)).position);

					if (BoundingBoxHelper::intersects_polygon(bounding_box, v_0_ws, v_1_ws, v_2_ws))
					{
						out->push_back(v_0_ws);
						out->push_back(v_1_ws);
						out->push_back(v_2_ws);
					}

					//check if this vertex is in bounding box
				}
			}
		}
	}

	return out;
}

std::vector<std::tuple<MeshCollider*, std::vector<vertex_array_filter>*>>* Scene::get_triangles_in_bounding_box(
	StructBoundingBox* bounding_box) const
{
	auto out = new std::vector<std::tuple<MeshCollider*, std::vector<vertex_array_filter>*>>;
	std::vector<Collider*> colliders_in_bb = *get_colliders_in_bounding_box(bounding_box);
	for (Collider* c : colliders_in_bb)
	{
		if (c->get_collider_type() == 1) // collider is a mesh collider
		{
			auto c_m = dynamic_cast<MeshCollider*>(c);
			auto v_f_array = new std::vector<vertex_array_filter>;
			for (int i = 0; i < c_m->get_vertex_arrays()->size(); i++) //get vertices of mesh collider
			{
				auto vertex_array = c_m->get_vertex_arrays()->at(i);
				auto v_f = new vertex_array_filter;
				for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3) //check every polygon if in bb
				{
					glm::vec3 v_0_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i)).position);
					glm::vec3 v_1_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i + 1)).position);
					glm::vec3 v_2_ws = c_m->transform_vertex_to_world_space(
						vertex_array->vertices->at(vertex_array->indices->at(i + 2)).position);

					if (BoundingBoxHelper::intersects_polygon(bounding_box, v_0_ws, v_1_ws, v_2_ws))
					{
						v_f->indices.push_back(i);
					}

					//check if this vertex is in bounding box
				}
				if (!v_f->indices.empty())
				{
					v_f->vertex_array_id = i;
					v_f_array->emplace_back(*v_f);
				}
			}
			if (!v_f_array->empty())
			{
				auto temp = std::make_tuple(c_m, v_f_array);
				out->emplace_back(temp);
			}
		}
	}

	return out;
}
