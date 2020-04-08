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

    int checkMouseOnMarker(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    void processMouseMovement(int numMarker, double xoffset, double yoffset);

private:
    //locations of the two markers
    glm::vec3 marker1;
    glm::vec3 marker2;

    //Rotation of the world
    glm::mat4 modelWorld;

    //current model matrix for the two markers
    glm::mat4 model_marker1;
    glm::mat4 model_marker2;

    //vertices and normals of the marker in STL file
    GLfloat *markervertices = NULL;
    GLfloat *markernormals = NULL;

    Shader markerShader;
    unsigned int markerVAO, markerVBO, markerNormalsVBO;
    //number of vertices in the STL file
    int markerIndex;
    void drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker);

    //methods to move marker through mouse
    int intersect(glm::vec3 rayOrigin, glm::vec3 rayDirection);

};