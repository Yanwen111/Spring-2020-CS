#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include <densityMap.h>
#include "shader.h"
#include "helper.h"

class MeasureObject {
public:
    MeasureObject();
    MeasureObject(DensityMap* gridPointer);
    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

    float* getSize();
    void setSize(float size);

    glm::vec3 getPosWorld(float freq, float vel, int depth);

    glm::vec3 getPos();
    void setPos(glm::vec3 inPos);

    void setIntersected(bool value);
    bool getIntersected();

    float getThreshold();
//    float getRsqVal();

    int checkMouseOnCube(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t);

    void calculateSphere();
    void calculateCylinder();
    void selectArea();

    //gets the radius in terms of actual object, not the grid space.
    float getRadius(float freq, float vel, int depth);

    //returns which object mode it's in
    int getDisplayObject();

    //return direction of cylinder
    glm::vec3 getDirection();

private:
    DensityMap* myGrid;

    glm::vec3 getCenter();
    float getRadius(glm::vec3 center);
    void findThreshold();
    glm::vec3 getDirection(glm::vec3 center);
    float getRadiusCylinder(glm::vec3 center, glm::vec3 direction);

    //number of iterations to go through to converge
    int N = 10000;
    float Nstop = 0.00001;

    //0 = sphere, 1 = cylinder
    int displayObject = 0;

    //vertices and normals of the marker in STL file
    GLfloat *cubevertices = NULL;
    GLfloat *cubenormals = NULL;

    //vertices and normals of the marker in STL file
    GLfloat *spherevertices = NULL;
    GLfloat *spherenormals = NULL;

    //vertices and normals of the marker in STL file
    GLfloat *cylindervertices = NULL;
    GLfloat *cylindernormals = NULL;

    bool showCube = true;

    float myThreshold = 150;

    glm::vec3 pos;
    glm::vec3 myColor;
    //width of box, radius of sphere, height of cylinder
    float mySize;
    float myCylinderRadius;
    //where the cylinder is pointing (unit vector)
    glm::vec3 upVector = glm::vec3(0,1,0);

    bool isIntersected = false;

    void cubeSetUp();
    void sphereSetUp();
    void cylinderSetUp();

    Shader cubeShader;
    unsigned int cubeVAO, cubeVBO, cubeNormalsVBO;
    //number of vertices in the STL file
    int cubeIndex;
    void drawCube(glm::mat4 projection, glm::mat4 view, glm::mat4 model_cube);
    glm::mat4 model;

    Shader sphereShader;
    unsigned int sphereVAO, sphereVBO, sphereNormalsVBO;
    //number of vertices in the STL file
    int sphereIndex;
    void drawSphere(glm::mat4 projection, glm::mat4 view, glm::mat4 model_cube);

    Shader cylinderShader;
    unsigned int cylinderVAO, cylinderVBO, cylinderNormalsVBO;
    //number of vertices in the STL file
    int cylinderIndex;
    void drawCylinder(glm::mat4 projection, glm::mat4 view, glm::mat4 model_cube);

    bool rayTriangleIntersect(
            const glm::vec3 &orig, const glm::vec3 &dir,
            const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
            float &t);
};