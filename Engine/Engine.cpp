#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Source/Core/Renderer/Shader/ShaderProgram.h"
#include "Source/Core/Renderer/Primitives/vertex_array.h"
#include "Source/Core/Scene/Object3D.h"
#include "Source/Core/Scene/Primitive3D/Mesh3D.h"
#include "Source/Core/Renderer/RenderContext.h"
#include "gtx/string_cast.hpp"
#include "Source/Core/Editor/GlobalContext.h"
#include "Source/Core/Renderer/Texture/texture_3d.h"
#include "Source/Core/Renderer/Texture/texture_2d.h"
#include "Source/Core/Scene/Camera3D.h"
#include "Source/Core/Scene/Handle.h"
#include "Source/Core/Scene/RayCast.h"
#include "Source/Core/Scene/Scene.h"
#include "Source/Util/AssetLoader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Source/Core/GUI/GUIManager.h"
#include "Source/Core/GUI/gui_performance_metrics.h"
#include "Source/Core/GUI/gui_scene_tools.h"
#include "Source/Core/GUI/LogGUI.h"
#include "Source/Core/GUI/ObjectInfo.h"
#include "Source/Core/GUI/SceneTree.h"
#include "Source/Core/PhysicsEngine/PhysicsEngine.h"
#include "Source/Core/Renderer/Primitives/quad_fill_screen.h"
#include "Source/Core/Renderer/Shader/compute_shader.h"
#include "Source/Core/Renderer/Texture/texture_buffer_object.h"
#include "Source/Core/Scene/DebugPrimitives/Line3D.h"
#include "Source/External/eventpp/include/eventpp/callbacklist.h"
#include "Source/Core/Scene/Lighting/SkyBox/sky_box_atmospheric_scattering.h"
#include "Source/Core/Scene/Lighting/SkyBox/sky_box_simple_sky_sphere.h"
#include "Source/Core/Scene/Modifiers/Implementations/Colliders/box_collider.h"
#include "Source/Core/Scene/Primitive3D/plane_3d.h"
#include "Source/Core/Scene/SceneTools/SurfelManagerOctree.h"
#define WINDOW_HEIGHT (1080/2)
#define WINDOW_WIDTH (1920/2)

#define KEY_AMOUNT 350
#define MOUSE_BUTTON_AMOUNT 8


//low level input callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void window_changed_callback(GLFWwindow* window, int width, int height);

//input processing
void processInput(Camera3D* camera3D, Scene* scene_context, GLFWwindow* window);


eventpp::CallbackList<void (glm::ivec2)> change_window_size_dispatcher;

bool keyPressed[KEY_AMOUNT];
bool keyClicked[KEY_AMOUNT];
bool keyReleased[KEY_AMOUNT];
bool mouseButtonsPressed[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsClicked[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsReleased[MOUSE_BUTTON_AMOUNT];

glm::vec2 mousePos;
glm::vec2 mouseNormalized;
auto windowSize = glm::ivec2(WINDOW_WIDTH,WINDOW_HEIGHT);
ImGuiIO* io = nullptr;

bool mouseCaptured;
glm::vec2 mouseLockPos; //to store the mouse position where the cursor is fixed

//TODO: this is temporary when there are multiple viewports this has to be managed dynamically 
Scene* scene = nullptr;
GUIManager* guiManager = nullptr;

//TEST: REMOVE ME
static rigid_body* rigid_body_mod;

//fence to sync CPU and GPU after each tick cycle
GLsync main_thread_fence;

class Importer;

int main()
{
	printf("Start");
	if (!glfwInit())
	{
		std::cerr << "Couldn't init GLFW\n";
		return -1;
	}

	//// -- CREATE RENDER CONTEXT AND WINDOW -- ////
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flo Engine", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cerr << "Couldn't create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GLFW_TRUE;
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Window init failed\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	// --- IMGUI INIT -- //

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();


	// --- INITIALIZE ENGINE BACKEND FEATURES --- //

	//Init Global Context
	auto global_context = GlobalContext();

	//compile default shader
	auto* default_shader = new ShaderProgram();
	default_shader->loadFromFile("EngineContent/Shader/DefaultShader.glsl");
	default_shader->compileShader();
	global_context.default_shader = default_shader;


	auto* default_color_shader = new ShaderProgram();
	default_color_shader->loadFromFile("EngineContent/Shader/DebugColorChangable.glsl");
	default_color_shader->compileShader();
	global_context.default_color_debug_shader = default_color_shader;

	auto light_pass_shader = new ShaderProgram();
	light_pass_shader->set_shader_header_include(DEFAULT_HEADERS, false);
	light_pass_shader->render_method = NONE;
	light_pass_shader->loadFromFile("EngineContent/Shader/DepthOnlyLightpassShader.glsl");
	light_pass_shader->compileShader();
	global_context.light_pass_depth_only_shader = light_pass_shader;

	global_context.global_primitives = {
		.cube = new Cube,
		.line = new Line,
		.plane = new primtitive_plane,
	};

	global_context.uniform_buffer_object = new uniform_buffer_object();

	//Inti Scene Context
	//scene root
	//init scene context
	scene = new Scene(&global_context, "Test Scene");
	auto scene_cam = new camera(WINDOW_WIDTH, WINDOW_HEIGHT, &change_window_size_dispatcher);
	//initialize render context
	auto editorRenderContext = new RenderContext{
		.camera = scene_cam,
		.default_shader = default_shader,
		.light_pass_depth_only_shader = light_pass_shader,
	};
	editorRenderContext->pass = render_pass_main;
	

	//register interaction callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, window_changed_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	auto editor3DCamera = new Camera3D(scene->get_root(), editorRenderContext);
	double renderFrameStart;


	/////// TEST STUF ///////


	//load textures 
	auto textureBase = new texture_2d;
	std::string pathTexture = "EngineContent/grass_base.png";
	textureBase->loadFromDisk(&pathTexture);

	auto textureNormal = new texture_2d;
	std::string pathTextureNormal = "EngineContent/grass_normal.png";
	textureNormal->loadFromDisk(&pathTextureNormal);


	//load models 
	auto plane = loadModel("EngineContent/Plane.fbx");
	plane->initializeVertexArrays();
	
	auto me_temple = loadModel("EngineContent/temple.fbx");
	me_temple->initializeVertexArrays();

	auto sphere = loadModel("EngineContent/Sphere.fbx");
	sphere->initializeVertexArrays();

	auto cube = loadModel("EngineContent/Cube.fbx");
	cube->initializeVertexArrays();

	auto me_test_building = loadModel("EngineContent/closedSpaceTest.fbx");
	me_test_building->initializeVertexArrays();
	
	auto me_test_landscape = loadModel("EngineContent/Landscape.fbx");
	me_test_landscape->initializeVertexArrays();

	auto me_test_triangle = loadModel("EngineContent/Triangle.fbx");
	me_test_triangle->initializeVertexArrays();

	auto me_inertia_test = loadModel("EngineContent/inertiaTest_mass_center.fbx");
	me_inertia_test->initializeVertexArrays();

	auto me_simple_building = loadModel("EngineContent/SimpleBuilding.fbx");
	me_simple_building->initializeVertexArrays();
	
	auto me_brutalist = loadModel("EngineContent/brutalist.fbx");
	me_brutalist->initializeVertexArrays();

	auto me_city = loadModel("EngineContent/city.fbx");
	me_city->initializeVertexArrays();

	//init shaders

	//auto* gaussian_gi_shader = new ShaderProgram();
	//gaussian_gi_shader->loadFromFile("EngineContent/Shader/GaussianGI.glsl");
	//gaussian_gi_shader->compileShader();


	auto* worldPosMat = new ShaderProgram();
	worldPosMat->loadFromFile("EngineContent/Shader/WorldPosition.glsl");
	worldPosMat->compileShader();
	
	auto* grey_shader = new ShaderProgram();
	grey_shader->loadFromFile("EngineContent/Shader/BasicGrey.glsl");
	grey_shader->compileShader();

	auto* green_shader = new ShaderProgram();
	green_shader->loadFromFile("EngineContent/Shader/BasicGreen.glsl");
	green_shader->compileShader();

	auto* emissive_shader = new ShaderProgram();
	emissive_shader->loadFromFile("EngineContent/Shader/BasicEmissive.glsl");
	emissive_shader->compileShader();
	

	/////ADD SCENE GEOMETRY:

	
	auto collision_test_cube_1 = new Mesh3D(scene->get_root(), cube);
	collision_test_cube_1->name = "collision_test_cube_1";
	collision_test_cube_1->set_position_global(0, -3, 0);
	collision_test_cube_1->add_modifier(new box_collider(collision_test_cube_1));

	auto rigid_body_test_cube_1 = new rigid_body(collision_test_cube_1);
	rigid_body_test_cube_1->gravity_enabled = false;
	rigid_body_test_cube_1->is_fixed = true;
	collision_test_cube_1->add_modifier(rigid_body_test_cube_1);

	auto collision_test_cube_2 = new Mesh3D(scene->get_root(), cube);
	collision_test_cube_2->name = "collision_test_cube_2";
	collision_test_cube_2->add_modifier(new box_collider(collision_test_cube_2));
	collision_test_cube_2->add_modifier(new rigid_body(collision_test_cube_2));
	

	auto sky_sphere = new sky_box_simple_sky_sphere(scene->get_root());
	//auto sky_sphere = new sky_box_atmospheric_scattering(scene->get_root());
	
	//TODO: this call should be automatically called when changing the scene
	scene->recalculate_from_root();

	//ADD DIRECT LIGHT
	auto direct_scene_light = new direct_light(scene->get_root(), 1024, 1024);
	direct_scene_light->setRotationLocal(-44, -9, 0);
	direct_scene_light->intensity = 1;
	direct_scene_light->set_light_settings(150,0.01,100,60);


	
	auto emissive_cube = new Mesh3D(scene->get_root(), cube);
	emissive_cube->name = "emissive_cube";
	emissive_cube->set_material(emissive_shader);

	auto object_plane = new Mesh3D(scene->get_root(), plane);
	object_plane->name = "plane_light";
	//object_plane->set_material(gaussian_gi_shader);
	object_plane->set_position_global(0, -4, 0);
	object_plane->setRotationLocal(-90, 0, 0);
	object_plane->setScale(50, 50, 1);

	auto object_house = new Mesh3D(scene->get_root(), me_test_building);
	object_house->name = "object_house";
	object_house->set_material(grey_shader);
	object_house->set_position_global(-12,-1.7,0);
	object_house->setRotationLocal(-90,0,0);

	auto object_landscape = new Mesh3D(scene->get_root(), me_test_landscape);
	object_landscape->name = "object_landscape";
	object_landscape->set_material(green_shader);
	object_landscape->set_position_global(-12,-1.7,0);
	object_landscape->setRotationLocal(-90,0,0);

	auto object_brutalist = new Mesh3D(scene->get_root(), me_brutalist);
	object_brutalist->name = "object_brutalist";
	object_brutalist->set_material(grey_shader);
	object_brutalist->set_position_global(-12,-40,40);
	object_brutalist->setRotationLocal(0,0,0);

	auto object_city = new Mesh3D(scene->get_root(), me_city);
	object_city->name = "object_city";
	object_city->set_material(grey_shader);
	object_city->set_position_global(-12,-20,40);
	object_city->setRotationLocal(-90,0,0);

	
	auto object_simple_building = new Mesh3D(scene->get_root(), me_simple_building);
	object_simple_building->name = "object_simple_building";
	object_simple_building->set_material(grey_shader);
	object_simple_building->set_position_global(-40,-5,0);
	object_simple_building->setRotationLocal(-90,-90,0);
	
	auto object_temple = new Mesh3D(scene->get_root(), me_temple);
	object_temple->name = "me_temple";
	object_temple->set_material(grey_shader);
	object_temple->set_position_global(40,-5,0);
	object_temple->setRotationLocal(-90,-90,0);


	//WIDNOWS
	//INTI GUI MANAGER
	guiManager = new GUIManager();
	guiManager->addGUI(new LogGUI(&global_context));
	guiManager->addGUI(new gui_performance_metrics(global_context.performance_metrics));
	guiManager->addGUI(new SceneTree(scene));
	guiManager->addGUI(new ObjectInfo(scene));
	guiManager->addGUI(new gui_scene_tools(scene));
	
	
	
	//INIT G buffer

	//albedo
    auto framebuffer_texture_albedo = new texture_2d();
    framebuffer_texture_albedo->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT,GL_LINEAR);

	//roughness metallic ao
    auto framebuffer_texture_roughness_metallic_ao = new texture_2d();
    framebuffer_texture_roughness_metallic_ao->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,GL_LINEAR);

	//normal
	auto framebuffer_texture_normal = new texture_2d();
	framebuffer_texture_normal->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGB16F, GL_RGB, GL_FLOAT,GL_LINEAR);

	//pos
	auto framebuffer_texture_ws = new texture_2d();
	framebuffer_texture_ws->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGB16F, GL_RGB, GL_FLOAT,GL_LINEAR);

	//emissive
	auto framebuffer_texture_emissive = new texture_2d();
	framebuffer_texture_emissive->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGB16F, GL_RGB, GL_FLOAT,GL_LINEAR);

	//g-buffer-flags
	auto framebuffer_render_flags = new texture_2d();
	framebuffer_render_flags->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT,GL_NEAREST);

	//depth
	auto framebuffer_texture_depth = new texture_2d();
	framebuffer_texture_depth->initialize_as_depth_map_render_target(windowSize.x, windowSize.y);

	auto g_buffer = framebuffer_object();
	g_buffer.attach_texture_as_color_buffer(framebuffer_texture_ws, 0); //THIS IS ALSO USED ELSEWHERE DON'T CHANGE ATTACHMENT IDS
	g_buffer.attach_texture_as_color_buffer(framebuffer_texture_normal, 1);
	g_buffer.attach_texture_as_color_buffer(framebuffer_texture_albedo, 2);
	g_buffer.attach_texture_as_color_buffer(framebuffer_render_flags, 3);
	g_buffer.attach_texture_as_color_buffer(framebuffer_texture_roughness_metallic_ao, 4);
	g_buffer.attach_texture_as_color_buffer(framebuffer_texture_emissive, 5);
	g_buffer.attach_texture_as_depth_buffer(framebuffer_texture_depth);


	//INIT surfel pass buffer
	
	//stores r,g,b surfel radiance_ambient a = 0 no surfel a = 1 surfel 
	auto framebuffer_surfel_pass_color = new texture_2d();
	framebuffer_surfel_pass_color->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT,GL_LINEAR);


	auto framebuffer_surfel_pass_metadata_0 = new texture_2d();
	framebuffer_surfel_pass_metadata_0->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT,GL_LINEAR);

	auto framebuffer_surfel_pass_metadata_1 = new texture_2d();
	framebuffer_surfel_pass_metadata_1->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGBA16F, GL_RGBA, GL_FLOAT,GL_LINEAR);

	auto surfel_buffer = framebuffer_object();
	surfel_buffer.attach_texture_as_color_buffer(framebuffer_surfel_pass_color, 0);
	surfel_buffer.attach_texture_as_color_buffer(framebuffer_surfel_pass_metadata_0, 1);
	surfel_buffer.attach_texture_as_color_buffer(framebuffer_surfel_pass_metadata_1, 2);
	surfel_buffer.clear_before_rendering = false;
	surfel_buffer.add_size_change_listener(&change_window_size_dispatcher);
	
	auto surfel_buffer_shader = new ShaderProgram();
	surfel_buffer_shader->loadFromFile("EngineContent/Shader/SurfelPass.glsl");
	surfel_buffer_shader->set_shader_header_include(DEFAULT_HEADERS, false);
	surfel_buffer_shader->render_method = NONE;
	surfel_buffer_shader->compileShader();
	surfel_buffer_shader->addTexture(framebuffer_texture_ws, "gPos");
	surfel_buffer_shader->addTexture(framebuffer_texture_normal, "gNormal");

	//final light pass buffer
	auto framebuffer_final_light_pass = new texture_2d();
	framebuffer_final_light_pass->initialize_as_frame_buffer(windowSize.x, windowSize.y, GL_RGB16F, GL_RGB, GL_FLOAT,GL_LINEAR);

	auto light_pass_buffer = framebuffer_object();
	light_pass_buffer.attach_texture_as_color_buffer(framebuffer_final_light_pass,0);
	light_pass_buffer.add_size_change_listener(&change_window_size_dispatcher);


	editorRenderContext->camera->set_render_target(&g_buffer);
	scene->init_surfel_manager(editor3DCamera, &surfel_buffer);

	auto pp_shader = new ShaderProgram();
	pp_shader->loadFromFile("EngineContent/Shader/PostProcessing.glsl");
	pp_shader->set_shader_header_include(DEFAULT_HEADERS, false);
	pp_shader->render_method = NONE;
	pp_shader->compileShader();
	pp_shader->addTexture(framebuffer_texture_ws, "gPos");
	pp_shader->addTexture(framebuffer_texture_normal, "gNormal");
	pp_shader->addTexture(framebuffer_texture_albedo, "gAlbedo");
	pp_shader->addTexture(framebuffer_texture_emissive, "gEmissive");
	pp_shader->addTexture(framebuffer_texture_depth, "dpeth_framebuffer");
	pp_shader->addTexture(framebuffer_texture_roughness_metallic_ao, "gRoughnessMetallicAo");
	pp_shader->addTexture(framebuffer_surfel_pass_color, "gSurfels");
	pp_shader->addTexture(framebuffer_surfel_pass_metadata_0, "surfel_framebuffer_metadata_0");
	pp_shader->addTexture(framebuffer_surfel_pass_metadata_1, "surfel_framebuffer_metadata_1");
	pp_shader->addTexture(framebuffer_render_flags, "gRenderFlags");
	pp_shader->addTexture(direct_scene_light->light_map(), "direct_light_map_texture");
	pp_shader->addTexture(framebuffer_final_light_pass, "light_pass_result");

	
	auto lighting_pass = new ShaderProgram();
	lighting_pass->loadFromFile("EngineContent/Shader/PBRLighting.glsl");
	lighting_pass->set_shader_header_include(DEFAULT_HEADERS, false);
	lighting_pass->render_method = NONE;
	lighting_pass->compileShader();
	lighting_pass->addTexture(framebuffer_texture_ws, "gPos");
	lighting_pass->addTexture(framebuffer_texture_normal, "gNormal");
	lighting_pass->addTexture(framebuffer_texture_albedo, "gAlbedo");
	lighting_pass->addTexture(framebuffer_texture_emissive, "gEmissive");
	lighting_pass->addTexture(framebuffer_texture_depth, "dpeth_framebuffer");
	lighting_pass->addTexture(framebuffer_texture_roughness_metallic_ao, "gRoughnessMetallicAo");
	lighting_pass->addTexture(framebuffer_surfel_pass_color, "gSurfels");
	lighting_pass->addTexture(framebuffer_surfel_pass_metadata_0, "surfel_framebuffer_metadata_0");
	lighting_pass->addTexture(framebuffer_surfel_pass_metadata_1, "surfel_framebuffer_metadata_1");
	lighting_pass->addTexture(framebuffer_render_flags, "gRenderFlags");
	lighting_pass->addTexture(direct_scene_light->light_map(), "direct_light_map_texture");
	
	auto quad_screen = new quad_fill_screen();
	quad_screen->load();

	//// ------ RENDER LOOP ------ ////
	while (!glfwWindowShouldClose(window))
	{
		renderFrameStart = glfwGetTime();
		global_context.performance_metrics->start_measuring(tick_cycle_time);

		glfwPollEvents(); //input events
		processInput(editor3DCamera, scene, window); //low level input processing

		//IMGUI 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//draw engine UI
		guiManager->tickGUI();


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear radiance_ambient buffer


		//PHYSICS
		//if handler is attached to rigid body
		//-> calculate velocity from dragging the object
		//-> suspend physics calculation for this object

		global_context.performance_metrics->start_measuring(physics);
		if (scene->handle()->is_attached())
		{
			auto modifiers = scene->handle()->attached_object_3d()->get_modifiers_by_id(10);
			for (auto modifier : modifiers)
			{
				auto r = dynamic_cast<rigid_body*>(modifier);
				if (r != nullptr)
				{
					glm::vec3 handler_delta = (scene->handle()->getWorldPosition() - scene->handle()->last_pos_) *
						static_cast<float>(1.0f / editorRenderContext->deltaTime);
					r->set_velocity(handler_delta);
					r->set_angular_momentum(glm::vec3());
					r->skip_next_step = true;
					scene->handle()->last_pos_ = scene->handle()->getWorldPosition();
					//printf("handler delta = %s\n",glm::to_string(handler_delta).c_str());
				}
			}
		}

		//run physics step
		scene->get_physics_engine()->evaluate_physics_step(editorRenderContext->deltaTime);
		
		global_context.performance_metrics->stop_and_store_measuring(physics);

		
		//TEST:
		pp_shader->recompile_if_changed();
		surfel_buffer_shader->recompile_if_changed();
		lighting_pass->recompile_if_changed();
		scene->get_surfel_manager()->compute_shader_approxmiate_ao->recompile_if_changed();
		scene->get_surfel_manager()->insert_surfel_compute_shader->recompile_if_changed();
		scene->get_surfel_manager()->compute_shader_find_least_shaded_pos->recompile_if_changed();


		//////////  LIGHT PASS :
		///
		global_context.performance_metrics->start_measuring(pass_direct_light);
		scene->light_pass(editor3DCamera->get_camera());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowSize.x, windowSize.y);
		global_context.performance_metrics->stop_and_store_measuring(pass_direct_light);

		
		//////////  MAIN PASS:
		global_context.performance_metrics->start_measuring(pass_main);
		editorRenderContext->camera->set_render_target(&g_buffer);
		editor3DCamera->calculateView();
		editorRenderContext->camera->use();

		//draw debug elements
		scene->draw_debug_tools(editorRenderContext);
		
		//draw scene elements
		scene->draw_scene(editorRenderContext);
		global_context.performance_metrics->stop_and_store_measuring(pass_main);


		//////////  SURFEL PASS:
		global_context.performance_metrics->start_measuring(pass_surfels);
		surfel_buffer.render_to_framebuffer();		
		surfel_buffer_shader->use();
		quad_screen->draw();
		global_context.performance_metrics->stop_and_store_measuring(pass_surfels);

		global_context.performance_metrics->start_measuring(surfels_tick);
		scene->get_surfel_manager()->tick();
		global_context.performance_metrics->stop_and_store_measuring(surfels_tick);

		//////////  FINAL LIGHT PASS AND POST PROCESSING
		///
		
		global_context.performance_metrics->start_measuring(pass_g_buffer);
		
		light_pass_buffer.render_to_framebuffer();
		lighting_pass->use();
		lighting_pass->set_uniform_vec3_f("cameraPosWs",glm::value_ptr(*scene_cam->getWorldPosition()));
		quad_screen->draw();


		
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glViewport(0, 0, windowSize.x, windowSize.y);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		//post-processing

		pp_shader->use();
		pp_shader->set_uniform_vec3_f("cameraPosWs", value_ptr(*editorRenderContext->camera->getWorldPosition()));
		pp_shader->setUniformMatrix4("projection_matrix", value_ptr(*editorRenderContext->camera->getProjection()));
		pp_shader->setUniformMatrix4("view_matrix", value_ptr(*editorRenderContext->camera->getView()));
		quad_screen->draw();
		glEnable(GL_DEPTH_TEST);
		global_context.performance_metrics->stop_and_store_measuring(pass_g_buffer);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//swap front and back buffer
		glfwSwapBuffers(window);

		//POST DRAW:
		scene->post_draw();

		//clear clicked / released arrays
		memset(keyClicked, 0, KEY_AMOUNT * sizeof(bool));
		memset(keyReleased, 0, KEY_AMOUNT * sizeof(bool));
		memset(mouseButtonsClicked, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));
		memset(mouseButtonsReleased, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));

		glFlush();
		glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);;
		GLenum result = glClientWaitSync(main_thread_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
			// Render thread has finished
		}
		glDeleteSync(main_thread_fence);
		editorRenderContext->deltaTime = glfwGetTime() - renderFrameStart;
		global_context.performance_metrics->stop_and_store_measuring(tick_cycle_time);

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_RELEASE:
		keyPressed[key] = false;
		keyReleased[key] = true;
		break;
	case GLFW_PRESS:
		keyPressed[key] = true;
		keyClicked[key] = true;
		break;
	default:
		break;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_RELEASE:
		mouseButtonsPressed[button] = false;
		mouseButtonsReleased[button] = true;
		break;
	case GLFW_PRESS:
		mouseButtonsPressed[button] = true;
		mouseButtonsClicked[button] = true;
		break;
	default:
		break;
	}

	io->AddMouseButtonEvent(button, action);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	mousePos = {xpos, ypos};
	mouseNormalized = {xpos / windowSize.x, ypos / windowSize.y};
	//std::printf("mouse pos: %f , %f ", xpos, ypos);
	io->AddMousePosEvent(xpos, ypos);
}

void window_changed_callback(GLFWwindow* window, int width, int height)
{
	windowSize = {width, height};
	//std::printf("window changed %i, %i", width, height);
	change_window_size_dispatcher(glm::ivec2(width, height));
}

#define CAMERA_SPEED 40 //TODO: make runtime changeable
#define CAMERA_ROTATION_SPEED 0.05

//TODO: generalize this especially mouse capture
void processInput(Camera3D* camera3D, Scene* scene_context, GLFWwindow* window)
{
	if (!io->WantCaptureMouse)
	{
		glm::vec2 cursorDelta;
		//capture mouse when right-clicking in window
		if (mouseButtonsClicked[GLFW_MOUSE_BUTTON_RIGHT])
		{
			mouseLockPos = mousePos;
			mouseCaptured = true;
		}

		if (mouseButtonsReleased[GLFW_MOUSE_BUTTON_RIGHT])
		{
			mouseLockPos = {0, 0};
			mouseCaptured = false;
		}

		if (mouseCaptured)
		{
			cursorDelta = mousePos - mouseLockPos;
			glfwSetCursorPos(window, mouseLockPos.x, mouseLockPos.y);
			camera3D->setRotationLocal(
				glm::vec3(glm::radians(cursorDelta.y * CAMERA_ROTATION_SPEED),
				          glm::radians(cursorDelta.x * CAMERA_ROTATION_SPEED), 0) + camera3D->getLocalRotation());
		}
		else
		{
			auto inverse_projection = inverse(*(camera3D->getRenderContext()->camera->getProjection()));
			auto inverse_view = camera3D->getGlobalTransform();

			auto ray_target = inverse_projection * glm::vec4(mouseNormalized.x * 2 - 1, -mouseNormalized.y * 2 + 1, 1.0,
			                                                 1.0);
			ray_target.w = 0;

			auto ray_direction = inverse_view * normalize(ray_target);
			auto ray_origin = camera3D->getWorldPosition();


			if (mouseButtonsReleased[GLFW_MOUSE_BUTTON_LEFT])
			{
				if (scene->handle()->is_moving_coord())
				{
					scene->handle()->editor_release_handle();
				}
				else
				{
					//if handle is active check first intersection with handle
					ray_cast_result a = scene->ray_cast_in_scene_unoptimized(
						ray_origin, ray_direction, 100000, VISIBILITY);
					if (a.hit)
					{
						scene->select_object(a.object_3d);
					}
					else
					{
						scene->deselect();
					}
				}
			}
			else if (mouseButtonsPressed[GLFW_MOUSE_BUTTON_LEFT])
			{
				if (scene->handle()->is_attached())
				{
					if (scene->handle()->is_moving_coord())
					{
						scene->handle()->editor_move_handle(ray_origin, ray_direction);
					}
					else
					{
						scene->handle()->editor_click_handle(ray_origin, ray_direction);
					}
				}
			}
			else if (mouseButtonsPressed[GLFW_MOUSE_BUTTON_MIDDLE])
			{
				rigid_body_mod->apply_force_ws(ray_direction, ray_origin, 100);
			}
		}
	}
	auto cameraInput = glm::vec2(0, 0);
	if (keyPressed[GLFW_KEY_W]) cameraInput += glm::vec2(1, 0);
	if (keyPressed[GLFW_KEY_S]) cameraInput += glm::vec2(-1, 0);
	if (keyPressed[GLFW_KEY_D]) cameraInput += glm::vec2(0, 1);
	if (keyPressed[GLFW_KEY_A]) cameraInput += glm::vec2(0, -1);

	if ((cameraInput.x > 0 || cameraInput.y > 0)) cameraInput = normalize(cameraInput);

	camera3D->setPositionLocal(

		camera3D->getForwardVector() *
		static_cast<float>(cameraInput.x * camera3D->getRenderContext()->deltaTime *
			CAMERA_SPEED) +
		camera3D->getRightVector() *
		static_cast<float>(cameraInput.y * camera3D->getRenderContext()->deltaTime *
			CAMERA_SPEED) +
		camera3D->getWorldPosition());
}
