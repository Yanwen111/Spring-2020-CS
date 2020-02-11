cmake_minimum_required(VERSION 3.15)
project(ethan_opengl)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

###################### GLAD ######################
include_directories(${PROJECT_SOURCE_DIR}/glad)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)

###################### OTHER ######################
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(ethan_opengl ${SOURCE_FILES} ${HEADER_FILES} glad/src/glad.c)

###################### GLM ######################
include_directories(${PROJECT_SOURCE_DIR}/glm)

###################### GLFW ######################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/glfw)

target_link_libraries(ethan_opengl PRIVATE glfw)