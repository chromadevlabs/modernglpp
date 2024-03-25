cmake_minimum_required(VERSION 3.19)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (default Debug)" FORCE)
endif()

add_subdirectory(deps/glfw)

project(GLAD)
add_library(GLAD STATIC deps/glad/src/glad.c)
target_include_directories(GLAD PRIVATE deps/glad/include)

project(modernglpp)
add_library(modernglpp STATIC modernglpp.cc)
target_include_directories(modernglpp PRIVATE deps/glad/include)
target_link_libraries(modernglpp PRIVATE GLAD)

project(example)
add_executable(example example.cc)
target_include_directories(example PRIVATE deps/glm)
target_link_libraries(example PRIVATE modernglpp glfw opengl32.lib)
