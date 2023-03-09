cmake_minimum_required(VERSION 3.24)
#project("Engine")

add_executable(Engine main.cpp Core/Renderer/Shader/ShaderProgram.h Core/Renderer/Shader/ShaderProgram.cpp Core/Renderer/renderable.h Core/Renderer/VertexArray.cpp Core/Scene/Object3D.h Core/Scene/Mesh.cpp Core/Scene/Mesh.h Core/Scene/Object3D.cpp Core/Renderer/RenderContext.h Core/Renderer/RenderContext.cpp Core/Scene/Camera3D.cpp Core/Scene/Camera3D.h)

target_link_libraries(Engine
        libglew_static
        glfw)