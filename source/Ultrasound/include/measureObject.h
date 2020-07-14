#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include "shader.h"
#include "helper.h"

class MeasureObject {
public:
    MeasureObject();
    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

    float getSize();
    void setSize(float size);

    glm::vec3 getPos();
    void setPos(glm::vec3 inPos);

    void setIntersected(bool value);
    bool getIntersected();

    int checkMouseOnCube(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t);

private:
    //vertices and normals of the marker in STL file
    GLfloat *cubevertices = NULL;
    GLfloat *cubenormals = NULL;

    glm::vec3 pos;
    glm::vec3 myColor;
    float mySize;

    bool isIntersected = false;

    void cubeSetUp();

    Shader cubeShader;
    unsigned int cubeVAO, cubeVBO, cubeNormalsVBO;
    //number of vertices in the STL file
    int cubeIndex;
    void drawCube(glm::mat4 projection, glm::mat4 view, glm::mat4 model_cube);
    glm::mat4 model_cube;

    bool rayTriangleIntersect(
            const glm::vec3 &orig, const glm::vec3 &dir,
            const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
            float &t);
};