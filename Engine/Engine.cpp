#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Source/Core/Renderer/Shader/ShaderProgram.h"
#include "Source/Core/Renderer/VertexArray.h"
#include "Source/Core/Scene/Object3D.h"
#include "Source/Core/Scene/Mesh3D.h"
#include "Source/Core/Renderer/RenderContext.h"
#include "gtx/string_cast.hpp"
#include "Source/Core/Scene/Camera3D.h"
#include "Source/Util/AssetLoader.h"

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920

#define KEY_AMOUNT 350
#define MOUSE_BUTTON_AMOUNT 8

//input callbacks
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);


void processInput(Camera3D *camera3D, GLFWwindow *window);

bool keyPressed[KEY_AMOUNT];
bool keyClicked[KEY_AMOUNT];
bool keyReleased[KEY_AMOUNT];
bool mouseButtonsPressed[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsClicked[MOUSE_BUTTON_AMOUNT];
bool mouseButtonsReleased[MOUSE_BUTTON_AMOUNT];
glm::vec2 mousePos;
bool mouseCaptured;
glm::vec2 mouseLockPos; //to store the mouse position where the cursor is fixed

class Importer;

int main() {

    //init glsl

    if (!glfwInit()) {
        std::cerr << "Couldnt init GLFW" << std::endl;
        return -1;
    }
    
    //create window 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);


    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "test", nullptr, nullptr);

    if (window == nullptr) {
        std::cerr << "Couldnt create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GLFW_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Window init failed" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    /////// TEST STUF ///////

    const char *t = "EngineContent/cubeArray.fbx";
    auto sphere = loadModel(t);
    sphere->initializeVertexArrays();
    

    auto *s = new ShaderProgram();
    //s->setShader(const_cast<char*>(fragmentShaderSource),
    //             const_cast<char*>(vertexShaderSource));

    s->loadFromFile("EngineContent/Shader/test.glsl");
    s->compileShader();

    auto material = new Material;
    material->shaderProgram = s;

    sphere->materials.push_back(material);

    auto root = new Object3D();


    auto m3D = new Mesh3D(sphere);
    m3D->setPositionLocal(0, 0, -5);
    root->addChild(m3D);


    ///////////////////////////////////////////////////////////////


    
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
    auto editor3DCamera = new Camera3D(&editorRenderContext);
    std::cout << glm::to_string(*editorRenderContext.camera.getProjection()) << std::endl;
    double renderFrameStart;
    while (!glfwWindowShouldClose(window)) {
        renderFrameStart = glfwGetTime();
        glfwPollEvents(); //input events
        processInput(editor3DCamera, window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear color buffer

        //std::cout<<glm::to_string(editor3DCamera->getForwardVector())<<std::endl;
        editor3DCamera->calculateView();
        //draw scene elements
        root->drawEntryPoint(&editorRenderContext);
        //swap front and back buffer
        glfwSwapBuffers(window);

        //clear clicked / released arrays
        memset(keyClicked, 0, KEY_AMOUNT * sizeof(bool));
        memset(keyReleased, 0, KEY_AMOUNT * sizeof(bool));
        memset(mouseButtonsClicked, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));
        memset(mouseButtonsReleased, 0, MOUSE_BUTTON_AMOUNT * sizeof(bool));

        editorRenderContext.deltaTime = glfwGetTime() - renderFrameStart;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (action) {
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

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    switch (action) {
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

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    mousePos = {xpos, ypos};
}

#define CAMERA_SPEED 10 //TODO: make runtime changeable

//TODO: generalize this especially mouse capture
void processInput(Camera3D *camera3D, GLFWwindow *window) {


    glm::vec2 cursorDelta;
    //capture mouse when right-clicking in window
    if (mouseButtonsClicked[GLFW_MOUSE_BUTTON_RIGHT]) {
        mouseLockPos = mousePos;
        mouseCaptured = true;
    }

    if (mouseButtonsReleased[GLFW_MOUSE_BUTTON_RIGHT]) {
        mouseLockPos = {0, 0};
        mouseCaptured = false;
    }

    if (mouseCaptured) {
        cursorDelta = mousePos - mouseLockPos;
        glfwSetCursorPos(window, mouseLockPos.x, mouseLockPos.y);
        camera3D->setRotationLocal(
                glm::vec3(glm::radians(cursorDelta.y), glm::radians(cursorDelta.x), 0) + camera3D->getLocalRotation());
    }

    glm::vec2 cameraInput = glm::vec2(0, 0);
    if (keyPressed[GLFW_KEY_W]) cameraInput += glm::vec2(1, 0);
    if (keyPressed[GLFW_KEY_S]) cameraInput += glm::vec2(-1, 0);
    if (keyPressed[GLFW_KEY_D]) cameraInput += glm::vec2(0, 1);
    if (keyPressed[GLFW_KEY_A]) cameraInput += glm::vec2(0, -1);

    if ((cameraInput.x > 0 || cameraInput.y > 0)) cameraInput = glm::normalize(cameraInput);

    camera3D->setPositionLocal(

            camera3D->getForwardVector() *
            static_cast<float >(cameraInput.x * camera3D->getRenderContext()->deltaTime *
                                CAMERA_SPEED) +
            camera3D->getRightVector() *
            static_cast<float >(cameraInput.y * camera3D->getRenderContext()->deltaTime *
                                CAMERA_SPEED) +
            camera3D->getWorldPosition());

}

