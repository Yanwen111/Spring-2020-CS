#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <fstream>
#include <iostream>
#include "shader.h"
#include "rotation.h"

#ifndef ETHAN_OPENGL_ROTATIONS_H
#define ETHAN_OPENGL_ROTATIONS_H

#endif //ETHAN_OPENGL_ROTATIONS_H

class Probe {
public:

    Probe(std::string filename);
    void openIMUFile(const std::string& inputFileName);

    //draws the probe to the screen
    void draw(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY);

    glm::mat4 getOrientation();
    glm::vec4 readNextLine();
    void closeFile();

private:
    unsigned int probeVAO, probeVBO, probeNormalsVBO;
    Shader probeShader;
    int probeindex;

    std::ifstream file;

    glm::vec4 parseLine(char *str);
};