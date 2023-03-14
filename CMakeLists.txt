cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)

project("Engine")

add_subdirectory(glew)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(Engine)
add_subdirectory(assimp)

set(BUILD_ASSIMP_TOOLS  ON)
set(ASSIMP_BUILD_STATIC_LIB ON)

target_include_directories(Engine PUBLIC glfw/include)
target_include_directories(Engine PUBLIC glew/include)
target_include_directories(Engine PUBLIC glm/glm)
target_include_directories(Engine PUBLIC glm/glm/gtc)
target_include_directories(Engine PUBLIC glm/glm/ext)
target_include_directories(Engine PUBLIC assimp/include/)

target_link_libraries(assimp)

