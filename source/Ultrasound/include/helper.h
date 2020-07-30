#include <string>
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>

namespace Helper{
    int read_stl(const std::string& file_name, GLfloat * &vertices, GLfloat * &normals);
}