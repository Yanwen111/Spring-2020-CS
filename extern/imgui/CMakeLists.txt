cmake_minimum_required(VERSION 3.15)

project(IMGUI)

include_directories(${PROJECT_SOURCE_DIR})
###################### GLAD ######################
include_directories(../../extern/glad)
include_directories(../../extern/glad/include)

file(GLOB CPP_FILES *.cpp)

add_compile_options(-O3)

add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} glfw)
