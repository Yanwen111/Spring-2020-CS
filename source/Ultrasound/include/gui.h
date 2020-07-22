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
#include "probe.h"
#include <boost/filesystem.hpp>

/**
 * GUI class handles all the GUI features on the screen and interactions with them
 */
class GUI {
public:
    GUI(GLFWwindow *window, const char* glsl_version, DensityMap* pointer, void (*setZoom)(int),
            bool (*readData)(DensityMap&, std::string, float, int, bool&, std::string&, int&, bool&),
            bool (*connectToProbe)(DensityMap&, std::string, std::string, std::string, std::string,
                    bool, int, int, int, int, int, int,
                    std::string, int, std::string&, bool&, bool&, std::string&
                    ),
            void (*setDepth)(int),
            void (*setGain)(float),
            bool (*saveFile)(bool, bool&, std::string&, bool)
            );

    void drawGUI(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY);
    void cleanUp();

    //returns int for which object is clicked
    int mouseClickedObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    void moveMarker(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen);

    bool mouseOnObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen);

    //if the glfw window is resized
    void setWidth(int inWidth);
    void setHeight(int inHeight);

private:
    void loadConfig();

    DensityMap* gridPointer;
    void (*setZoomMain)(int);
    bool (*readDataMain)(DensityMap&, std::string, float, int, bool&, std::string&, int&, bool&);
    bool (*connectToProbeMain)(DensityMap&, std::string, std::string, std::string, std::string,
                           bool, int, int, int, int, int, int,
                           std::string, int, std::string&, bool&, bool&, std::string&);
    void (*setGainMain)(float);
    void (*setDepthMain)(int);
    bool (*saveFileMain)(bool, bool&, std::string&, bool);

    //screen width and height
    int width;
    int height;

    //filepath of data folder
    boost::filesystem::path filePath;

    //GUI vars

    // which screen to render: 0 = opening, 1 = load 2 = scan
    int renderedScreen = 0;
    // whether data is loaded on grid.
    bool isDataLoaded = false;

    // which path user takes load file vs scan
    bool isLoadFile;

    //**************  Screen 0 (opening screen) vars  **********************************
    bool screen0Load = false;
    bool screen0Scan = false;

    //**************  Screen 1 (loadFile) vars  **********************************
    //1 = loading, 2 = loaded, 3 = read file error, 4 = no file selected error
    int screen1CurrState = 0;
    std::string screen1ErrorMessage;
    std::string screen1File;
    bool screen1Load = false;
    bool screen1Error = false;
    bool screen1DataUpdate = false;

    //**************  Screen 2 (scan probe) vars  **********************************
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

    //is true if connection failed
    bool screen2ErrorSetUp = false;
    //is true if saving file failed
    bool screen2ErrorSaveFile = false;
    //0 = user didn't click save file, 1 = success! -1 = failed to save file
    int screen2SaveFileState = 0;
    //Error message returned from attempting to connect
    std::string screen2ErrorMessage = "";

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

    //***********************  Display vars  ******************************************
    //display settings
    int dispDepth = 1100;
    float dispGain = 0.0;
    float dispWeight = 0.1;
    float dispBrightness = 0.559f;
    float dispContrast = 1.0f;
    int dispCutoff = 1;
    int dispZoom = 60;
    bool dispReset = false;
    //speed of sound
    int mediumActive = 1;
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

    //whether or not to enable snapping when moving markers
    bool snap = false;
    int snapThreshold = 130;

    std::vector<Marker> markers;

    //marker pair that mouse is on
    int intersectedMarkerIndex = -1;
    //marker within pair that mouse is on
    int intersectedMarkerNum;

    Marker* intersectedMarker = nullptr;

    Scale scale;
    Probe probe;

    bool isProbeLoaded = false;
    int probeType = 0; // 0 for submarine, 1 for white fin

    glm::mat4 modelWorld;

    //****************************  Render 2D Text Vars ************************************
    //Characters map needed for FreeType
    struct Character {
        unsigned int TextureID;  // ID handle of the glyph texture
        glm::ivec2   Size;       // Size of glyph
        glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
        unsigned int Advance;    // Offset to advance to next glyph
    };
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
    Shader textShader;

    //whether to display distance next to Marker
    bool showMarkerDistance;
    //marker positions in terms of screen coordinates
    float markerXPos;
    float markerYPos;

    static void setUp();
    void setUpFont();
    void drawWidgets(glm::mat4 projection, glm::mat4 view);
    void drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    void drawProbe(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY);
    static void render();
    void interactionHandler();
    void reset();

    void drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
    static double rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir);
    glm::vec3 getSnapPoint(glm::vec3 rayOrigin, glm::vec3 rayDirection);
    static bool intersectGrid(glm::vec3 rayOriginGrid, glm::vec3 rayDirectionGrid, float& tmin, float& tmax);
    glm::vec3 getSnapPointGrid(glm::vec3 p1, glm::vec3 p2, int numVals);
};