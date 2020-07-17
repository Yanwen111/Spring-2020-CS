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

    glm::vec3 getPos();
    void setPos(glm::vec3 inPos);

    void setIntersected(bool value);
    bool getIntersected();

    float getThreshold();
//    float getRsqVal();

    int checkMouseOnCube(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t);

    void calculate();
    void selectArea();

    //gets the radius in terms of actual object, not the grid space.
    float getRadius(float freq, float vel, int depth);

private:
//    struct PointCalc
//    {
//        float npoints;
//        float Xsum, Xsumsq, Xsumcube;
//        float Ysum, Ysumsq, Ysumcube;
//        float Zsum, Zsumsq, Zsumcube;
//        float XYsum, XZsum, YZsum;
//        float X2Ysum, X2Zsum, Y2Xsum, Y2Zsum, Z2Xsum, Z2Ysum;
//    };

    DensityMap* myGrid;

//    void prepareData(PointCalc& P);
//    void calculateLeastSquaresSphere(PointCalc P, float& A, float& B, float& C, float& Rsq);

    glm::vec3 getCenter();
    float getRadius(glm::vec3 center);
    void findThreshold();

//    float rsqVal;

    //number of iterations to go through to converge
    int N = 10000;
    float Nstop = 0.00001;

    //vertices and normals of the marker in STL file
    GLfloat *cubevertices = NULL;
    GLfloat *cubenormals = NULL;

    //vertices and normals of the marker in STL file
    GLfloat *spherevertices = NULL;
    GLfloat *spherenormals = NULL;

    bool showCube = true;

    float myThreshold = 150;

    glm::vec3 pos;
    glm::vec3 myColor;
    float mySize;

    bool isIntersected = false;

    void cubeSetUp();
    void sphereSetUp();

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
//    glm::mat4 model_cube;

    bool rayTriangleIntersect(
            const glm::vec3 &orig, const glm::vec3 &dir,
            const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
            float &t);
};