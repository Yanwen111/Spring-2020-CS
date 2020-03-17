#include <string>
#include "marker.h"

class GUI {

public:
    GUI(GLFWwindow* window, const char* glsl_version);
    void drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model, float rotationX, float rotationY);
    static void cleanUp();

    int getZoom();

    void setNumLinesDrawn(int num);
    void setTime(float time);
    void setQuaternion(glm::vec4 quatIn);
    void setEulerAngles(glm::vec3 eulerIn);
    int getThreshold();
    bool isReset;

private:
    float brightness;
    float gain;
    int threshold;
    double time;
    int numLines;
    int zoom;
    bool setMarker;

    glm::vec4 quat;
    glm::vec3 euler;

    glm::vec3 marker1;
    glm::vec3 marker2;
    Marker marker;

    static void setUp();
    void drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void render();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

};