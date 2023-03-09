#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Core/Renderer/Shader/ShaderProgram.h"
#include "Core/Renderer/VertexArray.h"
#include "Core/Scene/Object3D.h"
#include "Core/Scene/Mesh.h"
#include "Core/Renderer/RenderContext.h"
#include "gtx/string_cast.hpp"
#include "Core/Scene/Camera3D.h"

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

int main() {
    std::cout << "Hello, World!" << std::endl;
    if (!glfwInit()) {
        std::cerr << "Couldnt init GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);


    GLFWwindow *window = glfwCreateWindow(600, 600, "test", nullptr, nullptr);

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

    //test
    char *vertexShaderSource = "#version 330 core\n"
                               "layout (location = 0) in vec3 aPos;\n"
                               "uniform mat4 mMatrix;\n"
                               "uniform mat4 vMatrix;\n"
                               "uniform mat4 pMatrix;\n"
                               "void main()\n"
                               "{\n"
                               "vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);\n"
                               "gl_Position = pMatrix * vertexCamSpace; \n"
                               "}\0";

    char *fragmentShaderSource = "#version 330 core\n"
                                 "out vec4 FragColor;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                 "} ";

    float vertices[] = {
            1, 1, 0,  // top right
            1, -1, 0,  // bottom right
            -1, -1, 0,  // bottom left
            -1, 1, 0   // top left
    };

    float cube[] = {
            1.000000, 1.000000, -1.000000,
            1.000000, -1.000000, -1.000000,
            1.000000, 1.000000, 1.000000,
            1.000000, -1.000000, 1.000000,
            -1.000000, 1.000000, -1.000000,
            -1.000000, -1.000000, -1.000000,
            -1.000000, 1.000000, 1.000000,
            -1.000000, -1.000000, 1.000000
    };

    float a[] = {
            -0.999999, 10.653018, 1.000000,
            -0.999999, 10.653018, -1.000000,
            1.000001, 10.653018, -1.000000,
            10.653018, 1.000000, -1.000000,
            10.653018, -1.000000, -1.000000,
            10.653018, -1.000000, 1.000000,
            1.000001, 1.000000, 10.653018,
            1.000001, -1.000000, 10.653018,
            -0.999999, -1.000000, 10.653018,
            -10.699636, 1.000000, 0.067722,
            -10.699636, -1.000000, 0.067722,
            -10.525325, -1.000000, -1.924667,
            -1.000000, 1.000000, -10.653018,
            -1.000000, -1.000000, -10.653018,
            1.000000, -1.000000, -10.653018,
            -0.999998, -10.653018, -1.000000,
            -0.999998, -10.653018, 1.000000,
            1.000002, -10.653018, 1.000000
    };

    unsigned int aI[] = {
            1, 3, 2,
            4, 6, 5,
            7, 9, 8,
            10, 12, 11,
            13, 15, 14,
            16, 18, 17
    };

    unsigned int cubeVertices[] = {
            5, 3, 1,
            3, 8, 4,
            7, 6, 8,
            2, 8, 6,
            1, 4, 2,
            5, 2, 6,
            5, 7, 3,
            3, 7, 8,
            7, 5, 6,
            2, 4, 8,
            1, 3, 4,
            5, 1, 2
    };

    for (int i = 0; i < sizeof(aI) / sizeof(int); ++i) {
        aI[i] = aI[i] - 1;

    }
    for (int i = 0; i < sizeof(cubeVertices) / sizeof(int); ++i) {
        cubeVertices[i] = cubeVertices[i] - 1;

    }

    unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
    };


    auto *s = new ShaderProgram();
    s->setShader(fragmentShaderSource,
                 vertexShaderSource);
    s->compileShader();
    auto *v1 = new VertexArray(sizeof(cube) / sizeof(float), sizeof(cubeVertices) / sizeof(int), cube, cubeVertices, s);

    auto root = new Object3D();

    std::srand(0);

    //random 3D objects 
    auto m = new Mesh();
    m->setPositionLocal(5, 2, -10);
    m->setVertexArray(v1);
    root->addChild(m);
    std::cout << glm::to_string(m->getWorldPosition());

    auto editorRenderContext = RenderContext{
            *new Camera(600, 600)
    };

    //register interaction callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window,mouse_button_callback);


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
    glm::vec2 cameraInput = glm::vec2(0, 0);

    if (keyPressed[GLFW_KEY_W]) cameraInput += glm::vec2(1, 0);
    if (keyPressed[GLFW_KEY_S]) cameraInput += glm::vec2(-1, 0);
    if (keyPressed[GLFW_KEY_D]) cameraInput += glm::vec2(0, 1);
    if (keyPressed[GLFW_KEY_A]) cameraInput += glm::vec2(0, -1);

    if ((cameraInput.x > 0 || cameraInput.y > 0)) cameraInput = glm::normalize(cameraInput);

    camera3D->setPositionLocal(camera3D->getForwardVector() *
                               static_cast<float >(cameraInput.x * camera3D->getRenderContext()->deltaTime *
                                                   CAMERA_SPEED) +
                               camera3D->getWorldPosition());


    glm::vec2 cursorDelta;
    //capture mouse when right-clicking in window
    if (mouseButtonsClicked[GLFW_MOUSE_BUTTON_RIGHT]) {
        mouseLockPos = mousePos;
        mouseCaptured = true;
    }

    if (mouseButtonsReleased[GLFW_MOUSE_BUTTON_RIGHT]) {
        mouseLockPos = {0,0};
        mouseCaptured = false;
    }

    if (mouseCaptured) {
        glfwSetCursorPos(window,mouseLockPos.x, mouseLockPos.y);
    }
}

