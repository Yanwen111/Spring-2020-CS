#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "imgui.h"

#include <iostream>

#include <string>
#include <thread>
#include <chrono>

#include "camera.h"
#include "data.h"
#include "probe.h"
#include "gui.h"
#include "remote.h"

#define PI 3.141592653589

#define SCR_WIDTH 1000
#define SCR_HEIGHT 800

// Keyboard and mouse input functions
void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos);
void cursorPosRotationCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window);

// Demo functions to show what the volume map looks like
void sphereDemo(DensityMap& grid);
void fanDemo(DensityMap& grid);

// Used for multi-threads
void renderLoop(GLFWwindow* window, Probe& probe, DensityMap& grid, GUI& myGUI, std::string windowTitle);

bool dataUpdate = false;

// Used in the mouse movement callbacks
double lastMouseX;
double lastMouseY;
bool firstMouse;

float rotationX;
float rotationY;

bool mousePressed;
int guiObjectPressed;
double xposMarker, yposMarker;

// Creating a Camera object
Camera cam;
Probe probe;
GUI* myGUIpointer;
Socket soc((char*)"Linux");

int depth = 2500;

glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;

const bool ROTATE_GRID = true;

std::thread dataThread;

int main() {

    /* test remote codes */
    soc.getComputerIP();

    // Window title
    std::string windowTitle = "Density Map";
    // Initializing the OpenGL context
    glfwInit();
    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    // Creating the window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle.c_str(), NULL, NULL);

    // If the window is not created (for any reason)
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setting callbacks
    glfwMakeContextCurrent(window);

    if (ROTATE_GRID) {
        glfwSetCursorPosCallback(window, cursorPosRotationCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
    }
    else {
        glfwSetCursorPosCallback(window, cursorPosMovementCallback);
    }

    if (!ROTATE_GRID) {
        // Lock the cursor to the center of the window
        // and make it invisible
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // Load the OpenGL functions from the graphics card
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initializing mouse info
    lastMouseX = SCR_WIDTH / 2.0;
    lastMouseY = SCR_HEIGHT / 2.0;
    firstMouse = true;

    // Creating the density map
    int dim = 100;
    DensityMap grid(dim);

    //Create the GUI
    GUI myGUI(window, glsl_version, &grid);
    myGUIpointer = &myGUI;

    // Add all non-empty cells to the map
    grid.setThreshold(1);

    // Main event loop
    renderLoop(window, probe, grid, myGUI, windowTitle);

    myGUI.cleanUp();

    // GLFW cleanup
    glfwTerminate();
}

void renderLoop(GLFWwindow* window, Probe& probe, DensityMap& grid, GUI& myGUI, std::string windowTitle){

    // Variables for measuring FPS
    int numFrames = 0;
    double lastFPSUpdate = 0;

    int currProbe = -1;

    //The render loop to draw until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        double currentFrame = glfwGetTime();
        cam.deltaTime = currentFrame - cam.lastFrame;
        cam.lastFrame = currentFrame;

        // Self-explanatory
        processKeyboardInput(window);

        // Clears the screen and fills it a dark grey color
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Creating matrices to transform the vertices into NDC (screen) coordinates
        // between -1 and 1 that OpenGL can use
        projection = glm::perspective<float>(glm::radians(cam.fov), float(SCR_WIDTH) / SCR_HEIGHT, 0.01,
                                                       500.0);
        view = cam.getViewMatrix();
        model = glm::mat4(1.0);

        // Creates the model matrix for the grid based on the rotationX and rotationY values
        if (ROTATE_GRID) {
            model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
            model = glm::rotate(model, rotationX, glm::rotate(glm::vec3(1, 0, 0), rotationY, glm::vec3(0, -1, 0)));
        }

        //Checks if the user clicked the load button on the GUI
        if (myGUI.loadNew()) {
            //Creates the probe to load
            int newProbe = myGUI.getProbe();
            //probe is submarine
            if (newProbe == 0) {
                probe.loadNewProbe("config_file/models/PROBE_CENTERED.stl");
            }
            //probe is whiteFin
            if (newProbe == 1) {
                probe.loadNewProbe("config_file/models/WHITE_FIN_CENTERED.stl");
            }
            currProbe = newProbe;

            //stays false until the data processing thread is done processing the data
            dataUpdate = false;
            grid.clear();

            //read new file
            if (newProbe == 0) {
                //gets the file name, time gain value, depth,
                std::string file = myGUI.getFile();
                char *c = const_cast<char *>(file.c_str());
                float GAIN = myGUI.getGain(); /* 0 means no gain */
                depth = myGUI.getDepth();
                float updateCoefficient = myGUI.getUpdateCoefficient();

                grid.setUpdateCoefficient(updateCoefficient);
                //dataThread = std::thread(readDataSubmarine, std::ref(grid), c, GAIN, depth, std::ref(dataUpdate));
                dataThread = std::thread(readDataTest, std::ref(grid), c, GAIN, depth, std::ref(dataUpdate));
                dataThread.detach();
            }
            if (newProbe == 1) {
                std::string file = myGUI.getFile();
                char *c = &file[0];
                float GAIN = myGUI.getGain(); /* 0 means no gain */
                depth = myGUI.getDepth();
                float updateCoefficient = myGUI.getUpdateCoefficient();
                grid.setUpdateCoefficient(updateCoefficient);
                dataThread = std::thread(readDataWhitefin, std::ref(grid), c, GAIN, depth, std::ref(dataUpdate));
                dataThread.detach();
            }
            //Figure out how to rotate probe?
            //Add in error checking on GUI side later
        }

        if(currProbe != -1) {
            // Draw the probe
            probe.draw(projection, view, rotationX, rotationY);
        }

        //Set up GUI paramters
        myGUI.setNumLinesDrawn(getSamples());
        myGUI.setNumSamples(depth);
        myGUI.setVoxels(grid.getDim());
        myGUI.setFileSize(0);
        myGUI.setBrightness(grid.getBrightness());
        myGUI.setThreshold(grid.getThreshold());
        myGUI.setContrast(grid.getContrast());

        // Draw the density map and the surrounding cube
        grid.draw(projection, view, model);

        // Draw the GUI and set parameters
        myGUI.drawGUI(projection, view, model);
        myGUI.setTime(glfwGetTime());
        myGUI.setQuaternion(probe.getQuaternions());
        myGUI.setEulerAngles(probe.getEulerAngles());
        if (myGUI.isReset) {
            rotationX = 0;
            rotationY = 0;
        }
        if(dataUpdate){
            std::cout<<"Done Loading File"<<std::endl;
            myGUI.doneLoading();
            probe.openIMUFile("data/real_imu.txt");
            dataUpdate = false;
        }
        cam.fov = myGUI.getZoom();
        grid.setBrightness(myGUI.getBrightness());
        grid.setThreshold(myGUI.getThreshold());
        grid.setContrast(myGUI.getContrast());


        // Used to make camera move speed consistent
        cam.prevPos = cam.position;

        // Update the screen
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Measuring FPS
        if (glfwGetTime() - lastFPSUpdate >= 1) {
            std::string newTitle = windowTitle + " (" + std::to_string(numFrames) + " FPS)";
            glfwSetWindowTitle(window, newTitle.c_str());

            lastFPSUpdate = glfwGetTime();
            numFrames = 0;
        }

        // Increment the number of frames in the past second
        numFrames++;
    }
}

void processKeyboardInput(GLFWwindow *window) {
    // If shift is held down, then the camera moves faster
    bool sprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

    // WASD + Q and E movement
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (ROTATE_GRID) {
        if (glfwGetKey(window, GLFW_KEY_R)) {
            rotationX = 0;
            rotationY = 0;
        }
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_W))
            cam.processKeyboard(FORWARD, sprinting);
        if (glfwGetKey(window, GLFW_KEY_S))
            cam.processKeyboard(BACKWARD, sprinting);
        if (glfwGetKey(window, GLFW_KEY_A))
            cam.processKeyboard(LEFT, sprinting);
        if (glfwGetKey(window, GLFW_KEY_D))
            cam.processKeyboard(RIGHT, sprinting);
        if (glfwGetKey(window, GLFW_KEY_Q))
            cam.processKeyboard(DOWN, sprinting);
        if (glfwGetKey(window, GLFW_KEY_E))
            cam.processKeyboard(UP, sprinting);

        // Hold C to zoom in
        if (glfwGetKey(window, GLFW_KEY_C)) {
            cam.fov = 30;
        }
        else {
            cam.fov = 70;
        }
    }
}

void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    // Ensures that the camera faces forward on startup
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    // Updating the camera angle
    double xoffset = xpos - lastMouseX;
    double yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    cam.processMouseMovement(xoffset, yoffset);
}

void cursorPosRotationCallback(GLFWwindow* window, double xpos, double ypos) {
    // Ensures that the cube faces forward on startup
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    // Updating the camera angle
    double xoffset = xpos - lastMouseX;
    double yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (mousePressed) {

        rotationY += xoffset / 200.0;
        rotationX -= yoffset / 200.0;

        // 1.5 is a bit less than pi / 2
        if (rotationX > 1.5) {
            rotationX = 1.5;
        }

        if (rotationX < -1.5) {
            rotationX = -1.5;
        }
    }

    if(guiObjectPressed != -1) {
        glm::mat4 cameraToWorld = glm::mat4(1.0f);
        glm::vec4 rightH = glm::vec4(glm::normalize(cam.right),1);
        glm::vec4 upH = glm::vec4(glm::normalize(cam.worldUp), 1);
        cameraToWorld[0] = rightH;
        cameraToWorld[1] = upH;
        cameraToWorld[2] = glm::vec4(glm::normalize(glm::cross(cam.right, cam.worldUp)), 1);
        cameraToWorld[3] = glm::vec4(cam.position, 1);

        //Generate Ray
        float imageAspectRatio = (SCR_WIDTH+0.0f) / (SCR_HEIGHT+0.0f); // assuming width > height
        float Px = (2 * ((xpos + 0.5) / SCR_WIDTH) - 1) * tan(cam.fov / 2 * M_PI / 180) * imageAspectRatio;
        float Py = (1 - 2 * ((ypos + 0.5) / SCR_HEIGHT)) * tan(cam.fov / 2 * M_PI / 180);
        glm::vec4 rayOrigin = glm::vec4(0,0,0,1);
        glm::vec4 rayOriginWorld, rayPWorld;
        rayOriginWorld = cameraToWorld * rayOrigin;
        rayPWorld = cameraToWorld * glm::vec4(Px, Py, -1, 1);
        glm::vec3 rayDirection = rayPWorld - rayOriginWorld;
        rayDirection = glm::normalize(rayDirection);

        myGUIpointer->moveMarker(guiObjectPressed, rayOriginWorld, rayDirection);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

        //Check if the mouse is on any GUI objects (like markers)
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);

        glm::mat4 cameraToWorld = glm::mat4(1.0f);
        glm::vec4 rightH = glm::vec4(glm::normalize(cam.right),1);
        glm::vec4 upH = glm::vec4(glm::normalize(cam.worldUp), 1);
        cameraToWorld[0] = rightH;
        cameraToWorld[1] = upH;
        cameraToWorld[2] = glm::vec4(glm::normalize(glm::cross(cam.right, cam.worldUp)), 1);
        cameraToWorld[3] = glm::vec4(cam.position, 1);

        //Generate Ray
        float imageAspectRatio = (SCR_WIDTH+0.0f) / (SCR_HEIGHT+0.0f); // assuming width > height
        float Px = (2 * ((xpos + 0.5) / SCR_WIDTH) - 1) * tan(cam.fov / 2 * M_PI / 180) * imageAspectRatio;
        float Py = (1 - 2 * ((ypos + 0.5) / SCR_HEIGHT)) * tan(cam.fov / 2 * M_PI / 180);
        glm::vec4 rayOrigin = glm::vec4(0,0,0,1);
        glm::vec4 rayOriginWorld, rayPWorld;
        rayOriginWorld = cameraToWorld * rayOrigin;
        rayPWorld = cameraToWorld * glm::vec4(Px, Py, -1, 1);
        glm::vec3 rayDirection = rayPWorld - rayOriginWorld;
        rayDirection = glm::normalize(rayDirection);

        guiObjectPressed = myGUIpointer->mouseClickedObjects(rayOriginWorld, rayDirection);
        if(guiObjectPressed != -1){
            xposMarker = xpos;
            yposMarker = ypos;
        }
        else
            mousePressed = true;
    }

    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        guiObjectPressed = -1;
        mousePressed = false;
    }
}