cmake_minimum_required(VERSION 3.15)
project(Ultrasound_AIO)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

###################### Includes for main.cpp ######################
include_directories(${PROJECT_SOURCE_DIR}/extern/glad/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/imgui)
include_directories(${PROJECT_SOURCE_DIR}/extern/glm)
include_directories(${PROJECT_SOURCE_DIR}/source/Ultrasound/include)
include_directories(${PROJECT_SOURCE_DIR}/source/DensityMap)

file(GLOB SOURCE_FILES main.cpp ${PROJECT_SOURCE_DIR}/extern/glad/src/glad.c)

add_subdirectory(source/Ultrasound)
add_subdirectory(source/DensityMap)
add_subdirectory(extern/imgui)

add_executable(Ultrasound_AIO ${SOURCE_FILES})

###################### GLFW ######################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/extern/glfw)

###################### libssh ######################
find_package(libssh REQUIRED)

add_compile_options(-O3)

target_link_libraries(Ultrasound_AIO PRIVATE glfw ULTRASOUND DENSITYMAP IMGUI ssh)