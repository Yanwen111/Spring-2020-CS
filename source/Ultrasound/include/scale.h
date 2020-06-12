#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include "shader.h"
#include "helper.h"
#include <vector>
//
// Created by Hayun Chong on 3/24/20.
//

#ifndef ULTRASOUND_SCALE_H
#define ULTRASOUND_SCALE_H

#endif //ULTRASOUND_SCALE_H

class Scale{
public:
    Scale();
    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec2 yPos, glm::vec2 xPos, glm::vec2 zPos);
    void setMeasurements(double freqIn, double velIn, int depthIn);

private:

    GLfloat *cubevertices = NULL;
    GLfloat *cubenormals = NULL;
    double freq;
    double vel;
    int depth;
    Shader scaleShader;
    unsigned int scaleVAO, scaleVBO, scaleNormalsVBO;
    int cubeIndex;

    std::vector<int> numberIndex;
    std::vector<GLfloat*> numberVertices;
    std::vector<GLfloat*> numberNormals;
    unsigned int numberVAO[10], numberVBO[10], numberNormalsVBO[10];

    std::vector<float> linesPlacement;

    void drawNumberShader(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, glm::vec3 objColor, int number);

    void drawCubeShader(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, glm::vec3 objColor);
    void drawYScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, double positionX, double positionZ);
    void drawXScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, double positionY, double positionZ);
    void drawZScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, double positionX, double positionY);

    void scaleObj(glm::mat4& model_marker, float x, float y, float z);
    void rotate(glm::mat4& model_marker, glm::mat4 modelRot);
    void translate(glm::mat4& model_marker, glm::mat4 modelRot, glm::vec3 position);

};