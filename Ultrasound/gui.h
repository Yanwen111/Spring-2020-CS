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
    GUI(GLFWwindow *window, const char* glsl_version, DensityMap* pointer, void (*setZoom)(int),
            bool (*readData)(DensityMap&, std::string, float, int, bool&, std::string&),
            bool (*connectToProbe)(std::string, std::string, std::string, std::string,
                    bool, int, int, int, int, int, int,
                    std::string, int, std::string&, bool&
                    )
            );

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
    void (*setZoomMain)(int);
    bool (*readDataMain)(DensityMap&, std::string, float, int, bool&, std::string&);
    bool (*connectToProbeMain)(std::string, std::string, std::string, std::string,
                           bool, int, int, int, int, int, int,
                           std::string, int, std::string&, bool&);

    //GUI vars

    // which screen to render: 0 = opening, 1 = load 2 = scan
    int renderedScreen = 0;
    // whether data is loaded on grid.
    bool isDataLoaded = false;

    // which path user takes load file vs scan
    bool isLoadFile;

    //Screen 0 (opening screen) vars
    bool screen0Load = false;
    bool screen0Scan = false;

    //Screen 1 (loadFile) vars
    int screen1CurrState = 0;
    std::string screen1Error;
    std::string screen1File;
    bool screen1Load = false;
    bool screen1DataUpdate = false;

    //Screen 2 (scan probe) vars
    std::string screen2ProbeIP;
    std::string screen2ProbeUsername = "root";
    std::string screen2ProbePassword = "root";
    std::string screen2CompIP;
    bool screen2IsSub = false;
    bool screen2IsDefault = true;
    std::string screen2CustomCommand;
    float screen2LxMin = -180;
    float screen2LxMax = 180;
    int screen2LxRes = 200;
    float screen2ServoMin = -30;
    float screen2ServoMax = 30;
    int screen2ServoRes = 200;

    bool screen2LiveScan = false;
    bool screen2ScanToFile = false;
    bool screen2SendCustom = false;
    //0 = first, 1 = loading, 2 = success,
    // 3 = error connection,
    // 4 = probeIP error, 5 = probeUsername error, 6 = probePassword error
    // 7 = compIP error
    // 8 = lxMin error, 9 = lxMax error, 10 = lxMin > lxMax, 11 = lxRes error
    // 12 = servoMin error, 13 = servoMax error, 14 = servoMin > servoMax, 15 = servoRes error
    int screen2CurrState = 0;
    std::string screen2Output = "";
    bool screen2Connected = false;

    bool passErrorCheckingScreen2();


    //***********************Display vars******************************************
    //display settings
    int dispDepth = 1500;
    float dispGain = 0;
    float dispWeight = 1;
    float dispBrightness = 0.0f;
    float dispContrast = 1.0f;
    int dispCutoff = 1;
    int dispZoom = 70;
    bool dispReset = false;
    //speed of sound
    int mediumActive = 0;
    float dispVel;
    float dispFreq = 15.6;
    std::string inputVel;
    //scale
    float scaleXY = 1;
    float scaleXZ = 1;
    float scaleYX = 0;
    float scaleYZ = 1;
    float scaleZX = 0;
    float scaleZY = 1;


//    bool loading;
//    int depth;
//    float brightness;
//    float gain;
//    int threshold;
//    float contrast;
//    double time;
//    int numLines;
//    int zoom;
//    bool setMarker;
//
//    //whether or not to enable snapping when moving markers
    bool snap;
    int snapThreshold;
//
//    float updateCoefficient;
//
    float marker1x, marker1y, marker1z;
    float marker2x, marker2y, marker2z;
//
//    double velocity;
//    int numSamples;
//
//    char fileName[100] = {0};
//    int probeType;
//
//    bool newLoad;
//
//    int voxels;
//    double fileSize;
//    double frequency;
//
//    glm::vec4 quat;
//    glm::vec3 euler;
//
//    Marker marker;
//
    std::vector<Marker> markers;

    Scale scale;
//    //locations of where the scales are located
//    float scaleX1, scaleX2, scaleY1, scaleY2, scaleZ1, scaleZ2;
//
//    int mediumActive;
//    char currVelocity[10] = { 0 };
//
    glm::mat4 modelWorld;

    static void setUp();
    void drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    void drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static void render();
    void interactionHandler();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static double rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir);
    glm::vec3 getSnapPoint(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    static bool intersectGrid(glm::vec3 rayOriginGrid, glm::vec3 rayDirectionGrid, float& tmin, float& tmax);
    glm::vec3 getSnapPointGrid(glm::vec3 p1, glm::vec3 p2, int numVals);


};