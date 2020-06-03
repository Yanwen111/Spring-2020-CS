//
// Created by Hayun Chong on 5/27/20.
//

#ifndef ULTRASOUND_GUI2_H
#define ULTRASOUND_GUI2_H

#endif //ULTRASOUND_GUI2_H
#include <string>
#include <densityMap.h>
#include "marker.h"
#include "scale.h"

/**
 * GUI class handles all the GUI features on the screen and interactions with them
 */
class GUI {

public:
    GUI(GLFWwindow *window, const char* glsl_version, DensityMap* pointer);
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
    std::string getFile();
    int getDepth();
    float getUpdateCoefficient();

    void setBrightness(float value);
    void setGain(float value);
    void setThreshold(int value);
    void setContrast(float value);
    void setUpdateCoefficient(float value);

    //returns int for which object is clicked
    int mouseClickedObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    void moveMarker(int numMarker, glm::vec3 rayOrigin, glm::vec3 rayDirection);

    bool loadNew();
    int getProbe();

    void doneLoading();

private:
    DensityMap* gridPointer;

    bool loading;
    int depth;
    float brightness;
    float gain;
    int threshold;
    float contrast;
    double time;
    int numLines;
    int zoom;
    bool setMarker;

    //whether or not to enable snapping when moving markers
    bool snap;
    int snapThreshold;

    float updateCoefficient;

    float marker1x, marker1y, marker1z;
    float marker2x, marker2y, marker2z;

    double velocity;
    int numSamples;

    char fileName[100] = {0};
    int probeType;

    bool newLoad;

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
    static double rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir);
    glm::vec3 getSnapPoint(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    static bool intersectGrid(glm::vec3 rayOriginGrid, glm::vec3 rayDirectionGrid, float& tmin, float& tmax);
    glm::vec3 getSnapPointGrid(glm::vec3 p1, glm::vec3 p2, int numVals);

    ImFont* AddDefaultFont( float pixel_size );
    void DoFitTextToWindow(ImFont *font, const char *text);

};