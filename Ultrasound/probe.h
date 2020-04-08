#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <fstream>
#include <iostream>
#include "shader.h"
#include "rotation.h"
#include "helper.h"

#ifndef ETHAN_OPENGL_ROTATIONS_H
#define ETHAN_OPENGL_ROTATIONS_H

#endif //ETHAN_OPENGL_ROTATIONS_H

class Probe {
public:

    Probe();
    void loadNewProbe(std::string filename);
    void openIMUFile(const std::string& inputFileName);

    //draws the probe to the screen
    void draw(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY);

    glm::mat4 getOrientation();
    glm::vec4 getQuaternions();
    glm::vec3 getEulerAngles();
    glm::vec4 readNextLine();
    void closeFile();

private:
    unsigned int probeVAO, probeVBO, probeNormalsVBO;
    glm::vec4 quat = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    Shader probeShader;
    int probeindex;

    std::ifstream file;

    glm::vec4 parseLine(char *str);
};

