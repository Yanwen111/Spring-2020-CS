cmake_minimum_required(VERSION 3.15)
project(Ultrasound)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

###################### GLAD ######################
include_directories(${PROJECT_SOURCE_DIR}/Dependencies/glad)
include_directories(${PROJECT_SOURCE_DIR}/Dependencies/glad/include)

####################### IMGUI ######################
include_directories(${PROJECT_SOURCE_DIR}/imgui)

###################### OTHER ######################
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/DensityMap/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Ultrasound/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp)
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/DensityMap/*.h ${CMAKE_CURRENT_SOURCE_DIR}/Ultrasound/*.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h)

###################### TCP #####################
#link_directories(D:/Software/CLion/mingw64/x86_64-w64-mingw32/lib)
link_directories(${PROJECT_SOURCE_DIR}/Dependencies)
link_libraries(ws2_32)

add_executable(Ultrasound ${SOURCE_FILES} ${INCLUDE_FILES} ${PROJECT_SOURCE_DIR}/Dependencies/glad/src/glad.c)

###################### GLM ######################
include_directories(${PROJECT_SOURCE_DIR}/Dependencies/glm)
include_directories(${PROJECT_SOURCE_DIR}/Ultrasound)
include_directories(${PROJECT_SOURCE_DIR}/DensityMap)

###################### GLFW ######################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glfw)

add_compile_options(-O3)

target_link_libraries(Ultrasound PRIVATE glfw ws2_32)
