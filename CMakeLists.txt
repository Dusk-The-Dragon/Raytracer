cmake_minimum_required(VERSION 3.10)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include_directories(${GLM_INCLUDE_DIRS})

add_executable(raytracer main.cpp)

target_link_libraries(raytracer OpenGL::GL glfw glm::glm)