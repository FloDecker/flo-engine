#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Source/Core/Renderer/Shader/ShaderProgram.h"
#include "Source/Core/Renderer/Primitives/vertex_array.h"
#include "Source/Core/Scene/Object3D.h"
#include "Source/Core/Scene/Mesh3D.h"
#include "Source/Core/Renderer/RenderContext.h"
#include "gtx/string_cast.hpp"
#include "Source/Core/Editor/GlobalContext.h"
#include "Source/Core/Renderer/Texture/Texture3D.h"
#include "Source/Core/Renderer/Texture/Texture2D.h"
#include "Source/Core/Scene/Camera3D.h"
#include "Source/Core/Scene/Handle.h"
#include "Source/Core/Scene/RayCast.h"
#include "Source/Core/Scene/Scene.h"
#include "Source/Util/AssetLoader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Source/Core/GUI/GUIManager.h"
#include "Source/Core/GUI/ObjectInfo.h"
#include "Source/Core/GUI/SceneTree.h"
#include "Source/Core/PhysicsEngine/PhysicsEngine.h"
#include "Source/Core/Scene/DebugPrimitives/Line3D.h"
#include "Source/External/eventpp/include/eventpp/callbacklist.h"
#include "Source/Core/Scene/Collider.h"
#include "Source/Core/Scene/Lighting/SkyBox/sky_box_atmospheric_scattering.h"
#include "Source/Core/Scene/Lighting/SkyBox/sky_box_simple_sky_sphere.h"
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
	light_pass_shader->loadFromFile("EngineContent/Shader/DepthOnlyLightpassShader.glsl");
	light_pass_shader->compileShader();
	global_context.light_pass_depth_only_shader = light_pass_shader;
	
	global_context.debug_primitives = {
		.cube = new Cube,
		.line = new Line,
	};

	global_context.uniform_buffer_object = new uniform_buffer_object();

	//Inti Scene Context
	//scene root
	//init scene context
	scene = new Scene(&global_context, "Test Scene");
	auto scene_cam = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, &change_window_size_dispatcher);
	//initialize render context
	auto editorRenderContext = new RenderContext{
		scene_cam
	};

	//init physics engine
	auto physics_engine = new PhysicsEngine();

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
	auto textureBase = new Texture2D;
	std::string pathTexture = "EngineContent/grass_base.png";
	textureBase->loadFromDisk(&pathTexture);

	auto textureNormal = new Texture2D;
	std::string pathTextureNormal = "EngineContent/grass_normal.png";
	textureNormal->loadFromDisk(&pathTextureNormal);


	//load models 
	auto plane = loadModel("EngineContent/Plane.fbx");
	plane->initializeVertexArrays();

	auto sphere = loadModel("EngineContent/Sphere.fbx");
	sphere->initializeVertexArrays();

	auto cube = loadModel("EngineContent/Cube.fbx");
	cube->initializeVertexArrays();

	auto me_test_building = loadModel("EngineContent/closedSpaceTest.fbx");
	me_test_building->initializeVertexArrays();

	auto me_test_triangle = loadModel("EngineContent/Triangle.fbx");
	me_test_triangle->initializeVertexArrays();

	auto me_inertia_test = loadModel("EngineContent/inertiaTest_mass_center.fbx");
	me_inertia_test->initializeVertexArrays();

	//init shaders

	auto* normal_debug_shader = new ShaderProgram();
	normal_debug_shader->loadFromFile("EngineContent/Shader/NormalVisualizer.glsl");
	normal_debug_shader->compileShader();


	auto* textureMaterial = new ShaderProgram();
	textureMaterial->loadFromFile("EngineContent/Shader/test.glsl");
	textureMaterial->compileShader();
	textureMaterial->addTexture(textureBase, "textureBase");
	textureMaterial->addTexture(textureNormal, "textureNormal");

	auto* posMaterial = new ShaderProgram();
	posMaterial->loadFromFile("EngineContent/Shader/test2.glsl");
	posMaterial->compileShader();

	auto* triangle_visualizer_material = new ShaderProgram();
	triangle_visualizer_material->loadFromFile("EngineContent/Shader/TriangleVisualizer.glsl");
	triangle_visualizer_material->compileShader();

	auto* lightTestMaterial = new ShaderProgram();
	lightTestMaterial->loadFromFile("EngineContent/Shader/lightingTest.glsl");
	lightTestMaterial->set_shader_header_include(DYNAMIC_DIRECTIONAL_LIGHT,true);
	lightTestMaterial->set_shader_header_include(DYNAMIC_AMBIENT_LIGHT,true);
	lightTestMaterial->compileShader();


	auto* worldPosMat = new ShaderProgram();
	worldPosMat->loadFromFile("EngineContent/Shader/WorldPosition.glsl");
	worldPosMat->compileShader();

	

	plane->materials.push_back(lightTestMaterial);
	sphere->materials.push_back(lightTestMaterial);


	/////ADD SCENE GEOMETRY:

	auto mSphere1 = new Mesh3D(scene->get_root(), sphere);
	mSphere1->materials.push_back(worldPosMat);
	mSphere1->setPositionLocal(20, 0, 0);
	mSphere1->setRotationLocalDegrees(0, 0, 0);
	mSphere1->name = "sphere 1";

	auto mSphere2 = new Mesh3D(scene->get_root(), sphere);
	mSphere2->setPositionLocal(0, 0, 10);
	mSphere2->name = "sphere 2";

	auto test_triangle = new Mesh3D(scene->get_root(), me_test_triangle);
	test_triangle->setPositionLocal(0, 0, -20);
	test_triangle->name = "test_triangle";

	auto mSphere3 = new Mesh3D(mSphere2, sphere);
	mSphere3->setPositionLocal(0, 0, 5);
	mSphere3->name = "sphere 3";
	mSphere3->add_modifier(new physics_object_modifier(mSphere3, physics_engine));

	auto mInertiaTest = new Mesh3D(scene->get_root(), me_inertia_test);
	mInertiaTest->name = "mInertiaTest";

	//auto sky_sphere = new sky_box_simple_sky_sphere(scene->get_root());
	auto sky_sphere = new sky_box_atmospheric_scattering(scene->get_root());

	auto s = std::string("ENGINE_COLLIDER");
	auto collider_test = dynamic_cast<MeshCollider*>(mInertiaTest->get_child_by_tag(&s));
	rigid_body_mod = new rigid_body(mInertiaTest, physics_engine, collider_test);
	rigid_body_mod->mass = 2000;
	mInertiaTest->add_modifier(rigid_body_mod);
	auto i = collider_test->get_inertia_tensor();
	auto xx = (vec_x * i * vec_x);
	auto yy = (vec_y * i * vec_y);
	auto zz = (vec_z * i * vec_z);
	printf("%f, %f, %f", xx.x, xx.y, xx.z);
	printf("%f, %f, %f", yy.x, yy.y, yy.z);
	printf("%f, %f, %f", zz.x, zz.y, zz.z);


	auto mSphere_phyics_1 = new Mesh3D(scene->get_root(), sphere);
	mSphere_phyics_1->materials.push_back(worldPosMat);
	mSphere_phyics_1->setPositionLocal(10, 0, 0);
	mSphere_phyics_1->name = "mSphere_phyics_1";
	auto spring1 = new mass_spring_point(mSphere_phyics_1, physics_engine);
	spring1->is_fixed = true;
	mSphere_phyics_1->add_modifier(spring1);

	auto mSphere_phyics_2 = new Mesh3D(scene->get_root(), sphere);
	mSphere_phyics_2->materials.push_back(worldPosMat);
	mSphere_phyics_2->setPositionLocal(5, 0, 0);
	mSphere_phyics_2->name = "mSphere_phyics_2";
	auto spring2 = new mass_spring_point(mSphere_phyics_2, physics_engine);
	spring2->damp = 1;
	mSphere_phyics_2->add_modifier(spring2);


	auto mSphere_phyics_3 = new Mesh3D(scene->get_root(), sphere);
	mSphere_phyics_3->materials.push_back(worldPosMat);
	mSphere_phyics_3->setPositionLocal(5, 2, 0);
	mSphere_phyics_3->name = "mSphere_phyics_3";
	auto spring3 = new mass_spring_point(mSphere_phyics_3, physics_engine);
	spring3->damp = 1;
	mSphere_phyics_3->add_modifier(spring3);

	auto mSphere_phyics_4 = new Mesh3D(scene->get_root(), sphere);
	mSphere_phyics_4->materials.push_back(worldPosMat);
	mSphere_phyics_4->setPositionLocal(10, 2, 0);
	mSphere_phyics_4->name = "mSphere_phyics_4";
	auto spring4 = new mass_spring_point(mSphere_phyics_4, physics_engine);
	spring4->damp = 1;
	spring4->is_fixed = true;
	mSphere_phyics_4->add_modifier(spring4);


	physics_engine->add_spring(spring1, spring2, 200.0);
	physics_engine->add_spring(spring2, spring3, 200.0);
	physics_engine->add_spring(spring3, spring4, 200.0);
	physics_engine->add_spring(spring4, spring1, 200.0);
	physics_engine->add_spring(spring3, spring1, 200.0);
	physics_engine->add_spring(spring2, spring4, 200.0);


	{
		/*
		    auto plane1 = new Mesh3D(plane, &global_context);
		    plane1->setScale(6,6,6);
		    plane1->setPositionLocal(0, 0, 0);
		    plane1->setRotationLocalDegrees(-90, 0, 0);
		    root->addChild(plane1);
		    plane1->name = "plane 1";
		
		
		    auto cube1 = new Mesh3D(cube, &global_context);
		    mSphere1->addChild(cube1);
		    cube1->set_position_global(0, 10, 0);
		    cube1->name = "THE CUUUUBE1";
		
		    auto cube2 = new Mesh3D(cube, &global_context);
		    mSphere1->addChild(cube2);
		    cube2->set_position_global(1, 13, 1);
		    cube2->name = "THE CUUUUBE2";
		
		    auto cube3 = new Mesh3D(cube, &global_context);
		    mSphere1->addChild(cube3);
		    cube3->set_position_global(0, 17, 0);
		    cube3->name = "THE CUUUUBE3";
		
		    auto triangle = new Mesh3D(me_test_triangle, &global_context);
		    mSphere1->addChild(triangle);
		    triangle->set_position_global(0, 20, 0);
		    triangle->name = "triangle";
		    triangle->materials.push_back(triangle_visualizer_material);
		

		    auto test_building = new Mesh3D(me_test_building,&global_context);
		    root->addChild(test_building);
		    test_building->set_position_global(0,2,0);
		    test_building->setRotationLocalDegrees(90,0,0);
		    test_building->name = "Test Building";
		
		
		    //ADD LIGHTS
		    auto light1 = new PointLight(&global_context);
		    root->addChild(light1);
		
		    //TEST LINE
		    auto line_test = new Line3D(root,glm::vec3(0,0,0),glm::vec3(3,3,-3), &global_context);
		
		    //TEST CUBE
		    auto cube_test = new Cube3D(&global_context);
		    root->addChild(cube_test);
		    cube_test->set_position_global(0,0,0);
		    cube_test->setScale(8,8,8);
		
		    
		    //TEST 3D TEXTURE
		
		    /**
		    auto test_texture_3d = new Texture3D();
		    test_texture_3d->initalize_as_voxel_data({-4,-4,-4},{4,4,4},8);
		    //for (unsigned int x = 0 ;x< 8;x++)
		    //{
		    //    for (unsigned int y = 0 ;y< 8;y++)
		    //    {
		    //        for (unsigned int z = 0 ;z< 8;z++)
		    //        {
		    //            test_texture_3d->write_to_voxel_field(x%15,y%15,z%15,15,x,y,z);
		    //        }
		    //    }
		    //}
		    //test_texture_3d->write_to_voxel_field(15,15,15,15,0,0,0);
		
		    //test_texture_3d->initialize();
		
		    auto* m_gi_test_mater = new ShaderProgram();
		    //m_gi_test_mater->loadFromFile("EngineContent/Shader/VoxelGI.glsl");
		    m_gi_test_mater->loadFromFile("EngineContent/Shader/VoxelVisualizer.glsl");
		    m_gi_test_mater->compileShader();
		    m_gi_test_mater->addVoxelField(test_texture_3d,"voxelData");
		    plane1->materials.push_back(m_gi_test_mater);
		    test_building->materials.push_back(triangle_visualizer_material);
		
		    //TEST VOXELIZER
		    
		    //auto vox = new Voxelizer(&global_context, &scene_context);
		    //vox->setScale(4,4,4);
		    //root->addChild(vox);
		    //vox->set_position_global(0,0,0);
		    //vox->voxel_precision = 8;
		
		
		
		
		
		    //vox->recalculate();
		    //vox->load_into_voxel_texture_df(test_texture_3d);
		    //test_texture_3d->initialize();
		    ///////////////////////////////////////////////////////////////
		    */
	}

	auto engine_handler_arrow_model = loadModel("EngineContent/Arrow.fbx");
	engine_handler_arrow_model->initializeVertexArrays();

	auto handlertest = new Mesh3D(scene->get_root(), engine_handler_arrow_model);
	handlertest->setPositionLocal(20, 0, 0);
	handlertest->setRotationLocalDegrees(0, 0, 0);
	handlertest->name = "handlertest";
	handlertest->materials.push_back(normal_debug_shader);

	auto handlertest_2 = new Mesh3D(scene->get_root(), engine_handler_arrow_model);
	handlertest_2->setPositionLocal(15, 0, 0);
	handlertest_2->setRotationLocalDegrees(0, 0, 0);
	handlertest_2->name = "handlertest_2";
	handlertest_2->materials.push_back(normal_debug_shader);

	auto handlertest_3 = new Mesh3D(scene->get_root(), engine_handler_arrow_model);
	handlertest_3->setPositionLocal(10, 0, 0);
	handlertest_3->setRotationLocalDegrees(0, 0, 0);
	handlertest_3->name = "handlertest_2";
	handlertest_3->materials.push_back(normal_debug_shader);

	//TODO: this call should be automatically called when changing the scene
	scene->recalculate_from_root();

	//ADD DIRECT LIGHT
	auto direct_scene_light = new direct_light(scene->get_root(), 1024,1024);
	direct_scene_light->intensity = 1;


	//depth map visualizer
	auto* visualize_light_map = new ShaderProgram();
	visualize_light_map->loadFromFile("EngineContent/Shader/testVisualizeDepthMap.glsl");
	visualize_light_map->compileShader();
	visualize_light_map->addTexture(scene->get_scene_direct_light()->light_map(), "depthMap");

	auto visualizer_depth = new Mesh3D(scene->get_root(), plane);
	visualizer_depth->materials.push_back(visualize_light_map);

	
	//WIDNOWS
	//INTI GUI MANAGER
	guiManager = new GUIManager();
	guiManager->addGUI(new SceneTree(scene));
	guiManager->addGUI(new ObjectInfo(scene));
	scene->get_debug_tools()->draw_debug_line({0, 0, 0}, {0, 0, 20}, {1, 1, 1});
	
	//rigid_body_mod->apply_force_at_vertex(1, glm::vec3(100, 0, 0));
	
	//// ------ RENDER LOOP ------ ////
	while (!glfwWindowShouldClose(window))
	{
		renderFrameStart = glfwGetTime();
	
		//lightpass 
		scene->light_pass(); 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowSize.x, windowSize.y);
		
		glfwPollEvents(); //input events
		processInput(editor3DCamera, scene, window); //low level input processing

		//IMGUI 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//draw engine UI
		guiManager->tickGUI();


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear color buffer

		//run physics step
		physics_engine->evaluate_physics_step(editorRenderContext->deltaTime);

		//TEST:
		lightTestMaterial->recompile_if_changed();
		auto rot = glfwGetTime() * 10;
		//handlertest->setRotationLocalDegrees({rot,0,0});
		//handlertest_2->setRotationLocalDegrees({0,rot,0});
		//handlertest_3->setRotationLocalDegrees({0,0,rot});
		
		editor3DCamera->calculateView();
		//draw scene elements
		scene->draw_scene(editorRenderContext);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//swap front and back buffer
		glfwSwapBuffers(window);


		//clear clicked / released arrays
		memset(keyClicked, 0, KEY_AMOUNT * sizeof(bool));
		memset(keyReleased, 0, KEY_AMOUNT * sizeof(bool));
		memset(mouseButtonsClicked, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));
		memset(mouseButtonsReleased, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));

		editorRenderContext->deltaTime = glfwGetTime() - renderFrameStart;
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

#define CAMERA_SPEED 10 //TODO: make runtime changeable
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
					ray_cast_hit a = RayCast::ray_cast_editor(scene_context, ray_origin, ray_direction);
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
				printf("middleclik");
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
