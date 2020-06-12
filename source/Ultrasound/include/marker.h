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
    Marker(glm::vec3 color);
    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    float getDistance(float freq, float vel, int depth);
    void setPositionMarker1(glm::vec3 pos);
    void setPositionMarker2(glm::vec3 pos);

    glm::vec3 getMarker1Pos();
    glm::vec3 getMarker2Pos();

    int checkMouseOnMarker(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t);

    void setIntersected(int markerNum);

    void setHidden(bool val);
    bool getHidden();

private:
    //locations of the two markers
    glm::vec3 marker1;
    glm::vec3 marker2;

    //marker color
    glm::vec3 color;

    //Rotation of the world
    glm::mat4 modelWorld;

    //current model matrix for the two markers
    glm::mat4 model_marker1;
    glm::mat4 model_marker2;

    //-1 for no marker intersected, 1 for marker 1, 2 for marker 2
    int intersectedMarker = -1;

    //vertices and normals of the marker in STL file
    GLfloat *markervertices = NULL;
    GLfloat *markernormals = NULL;

    Shader markerShader;
    unsigned int markerVAO, markerVBO, markerNormalsVBO;
    //number of vertices in the STL file
    int markerIndex;
    void drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker, bool intersected);

    //methods to move marker through mouse
    int intersect(glm::vec3 rayOrigin, glm::vec3 rayDirection);

    //whether marker is drawn
    bool isHidden;

};