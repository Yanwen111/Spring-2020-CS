cmake_minimum_required(VERSION 3.15)

project(DENSITYMAP)

#include_directories(include)
###################### GLAD ######################
include_directories(../../extern/glad)
include_directories(../../extern/glad/include)

###################### GLM ######################
include_directories(../../extern/glm)
include_directories()

file(GLOB CPP_FILES *.cpp)

add_compile_options(-O3)

add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} glfw)