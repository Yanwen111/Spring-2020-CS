cmake_minimum_required(VERSION 3.15)

project(ULTRASOUND)

include_directories(include)
###################### GLAD ######################
include_directories(../../extern/glad)
include_directories(../../extern/glad/include)

####################### IMGUI ######################
include_directories(../../extern/imgui)

######################### FREETYPE ######################
#find_package(Freetype REQUIRED)
#include_directories(${FREETYPE_INCLUDE_DIRS})

####################### BOOST ######################
find_package(Boost 1.56 REQUIRED COMPONENTS date_time filesystem iostreams)

###################### GLM ######################
include_directories(../../extern/glm)
include_directories(../../source/DensityMap)

file(GLOB CPP_FILES source/*.cpp)

###################### libssh ######################
find_package(libssh REQUIRED)

add_compile_options(-O3)

add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

#target_link_libraries(${PROJECT_NAME} PRIVATE glfw DENSITYMAP ssh Boost::date_time Boost::filesystem Boost::iostreams ${FREETYPE_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw DENSITYMAP ssh Boost::date_time Boost::filesystem Boost::iostreams)