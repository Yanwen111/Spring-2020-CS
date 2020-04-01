#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include "shader.h"
#include "helper.h"

class Marker {
public:
    Marker();
    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    float getDistance(float freq, float vel, int depth);
    void setPositionMarker1(glm::vec3 pos);
    void setPositionMarker2(glm::vec3 pos);

    glm::vec3 getMarker1Pos();
    glm::vec3 getMarker2Pos();

private:
    glm::vec3 marker1;
    glm::vec3 marker2;
    GLfloat *markervertices = NULL;
    GLfloat *markernormals = NULL;
    Shader markerShader;
    unsigned int markerVAO, markerVBO, markerNormalsVBO;
    int markerIndex;
    void drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker);

};