#include <string>
#include "marker.h"
#include "scale.h"

class GUI {

public:
    GUI(GLFWwindow* window, const char* glsl_version);
    void drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model, float rotationX, float rotationY);
    static void cleanUp();

    int getZoom();

    void setNumLinesDrawn(int num);
    void setNumSamples(int num);
    void setTime(float time);
    void setQuaternion(glm::vec4 quatIn);
    void setEulerAngles(glm::vec3 eulerIn);
    void setVoxels(int size);
    void setFileSize(double size);
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

    float marker1x, marker1y, marker1z;
    float marker2x, marker2y, marker2z;

    double velocity;
    int numSamples;

    int voxels;
    double fileSize;
    double frequency;

    glm::vec4 quat;
    glm::vec3 euler;

    Marker marker;

    Scale scale;

    static void setUp();
    void drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    void drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void render();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

};