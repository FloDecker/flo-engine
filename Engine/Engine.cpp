#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Source/Core/Renderer/Shader/ShaderProgram.h"
#include "Source/Core/Renderer/VertexArray.h"
#include "Source/Core/Scene/Object3D.h"
#include "Source/Core/Scene/Mesh3D.h"
#include "Source/Core/Renderer/RenderContext.h"
#include "gtx/string_cast.hpp"
#include "Source/Core/Editor/GlobalContext.h"
#include "Source/Core/Renderer/Texture/Texture3D.h"
#include "Source/Core/Renderer/Texture/Texture2D.h"
#include "Source/Core/Scene/Camera3D.h"
#include "Source/Core/Scene/Collider.h"
#include "Source/Core/Scene/Handle.h"
#include "Source/Core/Scene/RayCast.h"
#include "Source/Core/Scene/SceneContext.h"
#include "Source/Core/Scene/DebugPrimitives/Cube3D.h"
#include "Source/Core/Scene/DebugPrimitives/Line3D.h"
#include "Source/Core/Scene/SceneTools/VoxelizerTools/Voxelizer.h"
#include "Source/Util/AssetLoader.h"

#define WINDOW_HEIGHT (1080/2)
#define WINDOW_WIDTH (1920/2)

#define KEY_AMOUNT 350
#define MOUSE_BUTTON_AMOUNT 8

//input callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


void processInput(Camera3D* camera3D, SceneContext* scene_context, GLFWwindow* window);

bool keyPressed[KEY_AMOUNT];
bool keyClicked[KEY_AMOUNT];
bool keyReleased[KEY_AMOUNT];
bool mouseButtonsPressed[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsClicked[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsReleased[MOUSE_BUTTON_AMOUNT];

glm::vec2 mousePos;
glm::vec2 mouseNormalized;

bool mouseCaptured;
glm::vec2 mouseLockPos; //to store the mouse position where the cursor is fixed

class Importer;

int main()
{
    
    if (!glfwInit())
    {
        std::cerr << "Couldnt init GLFW" << std::endl;
        return -1;
    }

    //create window 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "test", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Couldnt create window" << std::endl;
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
        std::cerr << "Window init failed" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    //Init Global Context
    GlobalContext global_context = GlobalContext();

    auto* default_shader = new ShaderProgram();
    default_shader->loadFromFile("EngineContent/Shader/DefaultShader.glsl");
    default_shader->compileShader();
    global_context.default_shader = default_shader;


    auto* default_color_shader = new ShaderProgram();
    default_color_shader->loadFromFile("EngineContent/Shader/DebugColorChangable.glsl");
    default_color_shader->compileShader();
    global_context.default_color_debug_shader = default_color_shader;
    //Inti Scene Context

    //scene root
    auto root = new Object3D(&global_context);
    //init scene context
    auto scene_context = SceneContext(&global_context, root);
    auto handle = new Handle(&global_context, root);
    global_context.handle = handle;

    /////// TEST STUF ///////
    root->addChild(handle);

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

    auto me_sky_sphere = loadModel("EngineContent/SkySphere.fbx");
    me_sky_sphere->initializeVertexArrays();

    auto me_test_building = loadModel("EngineContent/closedSpaceTest.fbx");
    me_test_building->initializeVertexArrays();

    //init shaders
    
    auto* textureMaterial = new ShaderProgram();
    textureMaterial->loadFromFile("EngineContent/Shader/test.glsl");
    textureMaterial->compileShader();
    textureMaterial->addTexture(textureBase, "textureBase");
    textureMaterial->addTexture(textureNormal, "textureNormal");

    auto* posMaterial = new ShaderProgram();
    posMaterial->loadFromFile("EngineContent/Shader/test2.glsl");
    posMaterial->compileShader();

    auto* lightTestMaterial = new ShaderProgram();
    lightTestMaterial->loadFromFile("EngineContent/Shader/lightingTest.glsl");
    lightTestMaterial->compileShader();


    auto* worldPosMat = new ShaderProgram();
    worldPosMat->loadFromFile("EngineContent/Shader/WorldPosition.glsl");
    worldPosMat->compileShader();

    
    auto* m_sky_sphere = new ShaderProgram();
    m_sky_sphere->loadFromFile("EngineContent/Shader/SkySphere.glsl");
    m_sky_sphere->compileShader();


    plane->materials.push_back(lightTestMaterial);
    sphere->materials.push_back(lightTestMaterial);


    auto mSphere1 = new Mesh3D(sphere, &global_context);
    mSphere1->materials.push_back(worldPosMat);
    mSphere1->setPositionLocal(20,0,0);
    mSphere1->setRotationLocalDegrees(0,0,0);
    //mSphere1->setScale(2,0.5,1);
    mSphere1->name = "sphere 1";
    root->addChild(mSphere1);

    auto mSphere2 = new Mesh3D(sphere, &global_context);
    mSphere2->setPositionLocal(0, 0, 10);
    mSphere1->addChild(mSphere2);
    mSphere2->name = "sphere 2";

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


    //auto o_sky_sphere = new Mesh3D(me_sky_sphere,&global_context);
    //o_sky_sphere->materials.push_back(m_sky_sphere);
    //o_sky_sphere->setScale(512,512,512);
    //root->addChild(o_sky_sphere);

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
    auto test_texture_3d = new Texture3D();
    test_texture_3d->initalize_as_voxel_data({-4,-4,-4},{4,4,4},16);
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
    test_building->materials.push_back(m_gi_test_mater);

    //TEST VOXELIZER
    
    auto vox = new Voxelizer(&global_context, &scene_context);
    vox->setScale(4,4,4);
    root->addChild(vox);
    vox->set_position_global(0,0,0);
    vox->voxel_precision = 16;


    //TODO: this call should be automatically called when changing the scene
    scene_context.recalculate_from_root();


    vox->recalculate();
    vox->load_into_voxel_texture_df(test_texture_3d);
    test_texture_3d->initialize();
    ///////////////////////////////////////////////////////////////
    
    //initialize render context
    auto editorRenderContext = RenderContext{
        *new Camera(WINDOW_WIDTH, WINDOW_HEIGHT)
    };

    //register interaction callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    auto editor3DCamera = new Camera3D(&editorRenderContext, &global_context);
    std::cout << glm::to_string(*editorRenderContext.camera.getProjection()) << std::endl;
    double renderFrameStart;
    while (!glfwWindowShouldClose(window))
    {
        renderFrameStart = glfwGetTime();
        glfwPollEvents(); //input events
        processInput(editor3DCamera, &scene_context, window);
        if(m_gi_test_mater->recompile_if_changed())
        {
            //TODO: textures should automatically be reassigned after recompiling during runtime
            m_gi_test_mater->addVoxelField(test_texture_3d,"voxelData");
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear color buffer
        
        
        editor3DCamera->calculateView();
        //draw scene elements

        auto p = cube1->getWorldPosition();
        line_test->set_positions(mSphere1->getWorldPosition(),p);

        
        root->drawEntryPoint(&editorRenderContext);
        //swap front and back buffer
        glfwSwapBuffers(window);

        //clear clicked / released arrays
        memset(keyClicked, 0, KEY_AMOUNT * sizeof(bool));
        memset(keyReleased, 0, KEY_AMOUNT * sizeof(bool));
        memset(mouseButtonsClicked, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));
        memset(mouseButtonsReleased, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));

        editorRenderContext.deltaTime = glfwGetTime() - renderFrameStart;
        //std::cout<<1/editorRenderContext.deltaTime<<"-";
    }
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
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    mousePos = {xpos, ypos};
    mouseNormalized = {xpos / WINDOW_WIDTH, ypos / WINDOW_HEIGHT};
}

#define CAMERA_SPEED 10 //TODO: make runtime changeable
#define CAMERA_ROTATION_SPEED 0.05

//TODO: generalize this especially mouse capture
void processInput(Camera3D* camera3D, SceneContext* scene_context, GLFWwindow* window)
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
        auto inverse_projection = glm::inverse(*(camera3D->getRenderContext()->camera.getProjection()));
        auto inverse_view = camera3D->getGlobalTransform();

        auto ray_target = inverse_projection * glm::vec4(mouseNormalized.x * 2 - 1, -mouseNormalized.y * 2 + 1, 1.0,
                                                         1.0);
        ray_target.w = 0;

        auto ray_direction = inverse_view * glm::normalize(ray_target);
        auto ray_origin = camera3D->getWorldPosition();
        auto handle = scene_context->get_global_context()->handle;

        if (mouseButtonsReleased[GLFW_MOUSE_BUTTON_LEFT])
        {
            if (handle->is_moving_coord())
            {
                handle->editor_release_handle();
            }
            else
            {
                //if handle is active check first intersection with handle
                RayCastHit a = RayCast::ray_cast_editor(scene_context, ray_origin, ray_direction);
                if (a.hit)
                {
                    handle->attach_to_object(a.object_3d);
                }
                else
                {
                    handle->detach();
                }
            }
        }
        else if (mouseButtonsPressed[GLFW_MOUSE_BUTTON_LEFT])
        {
            if (handle->is_attached())
            {
                if (handle->is_moving_coord())
                {
                    handle->editor_move_handle(ray_origin, ray_direction);
                }
                else
                {
                    handle->editor_click_handle(ray_origin, ray_direction);
                }
            }
        }
    }

    glm::vec2 cameraInput = glm::vec2(0, 0);
    if (keyPressed[GLFW_KEY_W]) cameraInput += glm::vec2(1, 0);
    if (keyPressed[GLFW_KEY_S]) cameraInput += glm::vec2(-1, 0);
    if (keyPressed[GLFW_KEY_D]) cameraInput += glm::vec2(0, 1);
    if (keyPressed[GLFW_KEY_A]) cameraInput += glm::vec2(0, -1);

    if ((cameraInput.x > 0 || cameraInput.y > 0)) cameraInput = glm::normalize(cameraInput);

    camera3D->setPositionLocal(

        camera3D->getForwardVector() *
        static_cast<float>(cameraInput.x * camera3D->getRenderContext()->deltaTime *
            CAMERA_SPEED) +
        camera3D->getRightVector() *
        static_cast<float>(cameraInput.y * camera3D->getRenderContext()->deltaTime *
            CAMERA_SPEED) +
        camera3D->getWorldPosition());
}
