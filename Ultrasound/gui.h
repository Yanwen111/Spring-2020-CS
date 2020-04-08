#include <string>
#include "marker.h"
#include "scale.h"

class GUI {

public:
    GUI(GLFWwindow *window, const char* glsl_version);
    void drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void cleanUp();

    int getZoom();

    void setNumLinesDrawn(int num);
    void setNumSamples(int num);
    void setTime(float time);
    void setQuaternion(glm::vec4 quatIn);
    void setEulerAngles(glm::vec3 eulerIn);
    void setVoxels(int size);
    void setFileSize(double size);
    bool isReset;

    float getBrightness();
    float getGain();
    int getThreshold();
    float getContrast();

    void setBrightness(float value);
    void setGain(float value);
    void setThreshold(int value);
    void setContrast(float value);

    //returns int for which object is clicked
    int mouseClickedObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    void moveMarker(int numMarker, double xoffset, double yoffset);

private:
    float brightness;
    float gain;
    int threshold;
    float contrast;
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
    //locations of where the scales are located
    float scaleX1, scaleX2, scaleY1, scaleY2, scaleZ1, scaleZ2;

    int mediumActive;
    char currVelocity[10] = { 0 };

    glm::mat4 modelWorld;

    static void setUp();
    void drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    void drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void render();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

};