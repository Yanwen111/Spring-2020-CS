#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <iostream>
#include <string>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "data.h"
#include "probe.h"
#include "gui.h"


#define PI 3.141592653589

#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080

// Keyboard and mouse input functions
void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos);
void cursorPosRotationCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window);

// Demo functions to show what the volume map looks like
void sphereDemo(DensityMap& grid);
void fanDemo(DensityMap& grid);
void realDemo(DensityMap& grid);


// Used in the mouse movement callbacks
double lastMouseX;
double lastMouseY;
bool firstMouse;

float rotationX;
float rotationY;

bool mousePressed;

// Creating a Camera object
Camera cam;

const bool ROTATE_GRID = true;

int main() {
    // Window title
    std::string windowTitle = "Density Map";

    // Variables for measuring FPS
    int numFrames = 0;
    double lastFPSUpdate = 0;

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
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
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
    int dim = 101;
    DensityMap grid(dim);

    // Add a sphere to the center of the grid
    //fanDemo(grid);
    //sphereDemo(grid);
    realDemo(grid);
    gainControl(grid, 0); // 0 to something

    // Creating the probe
    Probe probe("data/PROBE_CENTERED.stl");
    // Open the IMU file for reading
    probe.openIMUFile("data/real_imu.txt");

    //Create the GUI
    GUI myGUI(window, glsl_version);

    // Add all non-empty cells to the map
    grid.setThreshold(1);
    grid.updateVertexBuffers();

    // Main event loop
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
        glm::mat4 projection = glm::perspective<float>(glm::radians(cam.fov), float(SCR_WIDTH) / SCR_HEIGHT, 0.01, 500.0);
        glm::mat4 view = cam.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0);

        if (ROTATE_GRID) {
            model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
            model = glm::rotate(model, rotationX, glm::rotate(glm::vec3(1, 0, 0), rotationY, glm::vec3(0, -1, 0)));
        }

        // Draw the probe
        probe.draw(projection, view, rotationX, rotationY);

        // Draw the density map and the surrounding cube
        grid.draw(projection, view, model);

        // Draw the GUI and set parameters
        myGUI.drawGUI(projection, view, model);
        myGUI.setNumLinesDrawn(100);
        myGUI.setTime(glfwGetTime());
        myGUI.setQuaternion(probe.getQuaternions());
        myGUI.setEulerAngles(probe.getEulerAngles());
        if(myGUI.isReset){
            rotationX = 0;
            rotationY = 0;
        }
        cam.fov = myGUI.getZoom();

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

    myGUI.cleanUp();

    // GLFW cleanup
    glfwTerminate();
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
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mousePressed = true;
    }

    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mousePressed = false;
    }
}

void sphereDemo(DensityMap& grid) {
    // Adds a sphere to the center of the volume map

    int dim = grid.getDim();

    float radius = 0.3;

    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < dim; k++) {
                float xd = i - ((dim - 1) / 2.0);
                float yd = j - ((dim - 1) / 2.0);
                float zd = k - ((dim - 1) / 2.0);

                float mxd = (dim - 1) / 2.0;
                float myd = (dim - 1) / 2.0;
                float mzd = (dim - 1) / 2.0;

                float distance = sqrt(xd * xd + yd * yd + zd * zd);
                float maxDistance = sqrt(mxd * mxd + myd * myd + mzd * mzd);
                float shade = (maxDistance - distance) / maxDistance;
                shade = shade * shade;

                if (distance < radius * dim) {
                    grid.cells[i][j][k] = static_cast<unsigned char>(shade * 255);
                }
            }
        }
    }
}

void fanDemo(DensityMap& grid) {
    // Adds a fan shape to the volume map
    // using the DensityMap::addLine() function

    glm::vec3 vertex = { 0.5, 0.5, 0.5 };

    float a1 = 1;
    float a2 = 1;

    float r = 0.3;

    for (; a2 <= 3; a2 += 0.01) {
        float x = r * sin(a1) * cos(a2);
        float y = r * sin(a1) * sin(a2);
        float z = r * cos(a1);

        std::vector<unsigned char> vals;

        for (int i = 0; i < 1000; i++) {
            vals.push_back(255);
        }

        grid.addLine(vertex, vertex + glm::vec3(x, y, z), vals);
    }
}

void realDemo(DensityMap& grid)
{
    //read the data from current red pitaya 2d data.
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;

    file_bytes = readFile("data/largemarble_2.txt");
    /* find all marker locations */
    marker_locations = find_marker(file_bytes);
    /* convert file bytes to data struct */
    file_to_data(file_bytes, marker_locations, scan_data);
    printf("the size of scan_data is %d\n", scan_data.size());
    /* convert data to vertex on screen */
    data_to_pixel(scan_data, line_data);
    printf("find the screen_data\n");


    float ddim = (float)grid.getDim();
    int len = line_data[0].vals.size();
    std::vector<float> maX, maY, maZ;  //assume the max/min coordinate must belong to edge points
    for (auto s: line_data) {
        maX.push_back(s.p1.x);
        maX.push_back(s.p2.x);
        maY.push_back(s.p1.y);
        maY.push_back(s.p2.y);
        maZ.push_back(s.p1.z);
        maZ.push_back(s.p2.z);
    }
    float maxx = *std::max_element(maX.begin(), maX.end());
    float minx = *std::min_element(maX.begin(), maX.end());
    float maxy = *std::max_element(maY.begin(), maY.end());
    float miny = *std::min_element(maY.begin(), maY.end());
    float maxz = *std::max_element(maZ.begin(), maZ.end());
    float minz = *std::min_element(maZ.begin(), maZ.end());
    float scale = ddim / len;
    printf("%f\n", scale);

    for (auto l: line_data)
    {
        glm::vec3 ps = {(l.p1.x-minx)/(maxx-minx), (l.p1.y-miny)/(maxy-miny), (l.p1.z-minz)/(maxz-minz)};
        glm::vec3 pe = {(l.p2.x-minx)/(maxx-minx), (l.p2.y-miny)/(maxy-miny), (l.p2.z-minz)/(maxz-minz)};
        grid.addLine(ps, pe, l.vals);
    }

/*
    //fill the cell
    double maxx = -999, maxy = -999, maxz = -999, minx = 999, miny = 999, minz = 999;

    for (auto s: screen_data) {
        maxx = std::max(s.X, maxx);
        maxy = std::max(s.Y, maxy);
        maxz = std::max(s.Z, maxz);
        minx = std::min(s.X, minx);
        miny = std::min(s.Y, miny);
        minz = std::min(s.Z, minz);

        //maxi = std::max(s.I, maxi);
        //mini = std::min(s.I, mini);
    }
    printf("find the maxes and mins\n");
    //printf("maxI is %f, min I is %f\n", maxi, mini);
    std::vector<std::vector<std::vector<int>>> cnts;// = grid.cells; //101x101x101
    //printf("the cnts size is x: %d, y: %d, z:%d\n", cnts.size(), cnts[0].size(), cnts[0][0].size());
    int ddim = grid.getDim();
    for (int i = 0; i < ddim; ++i) {
        cnts.push_back(std::vector<std::vector<int>>{});

        for (int j = 0; j < ddim; ++j) {
            cnts.back().push_back(std::vector<int>{});

            for (int k = 0; k < ddim; ++k) {
                cnts.back().back().push_back(0);
            }
        }
    }
    for (auto s: screen_data) {
        int tx = (int) ((s.X - minx) / (maxx - minx) * ddim);
        int ty = (int) ((s.Y - miny) / (maxy - miny) * ddim);
        //int tz = (int) ((s.Z - minz) / (maxz-minz) * ddim); //we don't have 3D data yet
        int tz = ddim / 2;
        if (tx < 0 || tx >= ddim || ty < 0 || ty >= ddim || tz < 0 || tz >= ddim)
            continue;
        grid.cells[tx][ty][tz] =
                cnts[tx][ty][tz] == 0 ? static_cast<unsigned char>(s.I * 255) : static_cast<unsigned char>(
                        (cnts[tx][ty][tz] * (int) grid.cells[tx][ty][tz] + s.I * 255) / (cnts[tx][ty][tz] + 1));
        //grid.cells[tx][ty][tz] = static_cast<unsigned char>(s.I*255);
        cnts[tx][ty][tz]++;
        //printf("the I at X: %d, Y:%d,  Z:%d   is: %f\n", tx, ty, tz, s.I);
    }
*/
}

