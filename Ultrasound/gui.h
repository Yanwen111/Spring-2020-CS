#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include <shader.h>

class GUI {

public:
    GUI(GLFWwindow* window, const char* glsl_version);
    void drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void cleanUp();

    int getZoom();

    void setNumLinesDrawn(int num);
    void setTime(float time);
    void setQuaternion(glm::vec4 quatIn);
    void setEulerAngles(glm::vec3 eulerIn);
    bool isReset;

private:
    float brightness;
    float gain;
    float cutoff;
    double time;
    int numLines;
    int zoom;
    bool setMarker;

    glm::vec4 quat;
    glm::vec3 euler;

    glm::vec3 marker1;
    glm::vec3 marker2;
//    GLfloat *markervertices = NULL;
//    GLfloat *markernormals = NULL;
//    Shader markerShader;
//    unsigned int markerVAO, markerVBO, markerNormalsVBO;
//    int markerIndex;

    static void setUp();
    void drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void render();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

};