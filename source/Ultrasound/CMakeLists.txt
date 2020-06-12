cmake_minimum_required(VERSION 3.15)

project(ULTRASOUND)

include_directories(include)
###################### GLAD ######################
include_directories(../../extern/glad)
include_directories(../../extern/glad/include)

####################### IMGUI ######################
include_directories(../../extern/imgui)

###################### GLM ######################
include_directories(../../extern/glm)
include_directories(../../source/DensityMap)

file(GLOB CPP_FILES source/*.cpp)

add_compile_options(-O3)

add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw DENSITYMAP)