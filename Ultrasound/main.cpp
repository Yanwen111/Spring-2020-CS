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

#include <mutex>
#include <thread>
#include <chrono>

#include "shader.h"
#include "camera.h"
#include "data.h"
#include "probe.h"
//#include "fakeData.h"


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
void renderLoop(GLFWwindow* window, Probe& probe, DensityMap& grid, std::string windowTitle);

bool dataUpdate = false;

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

void func1(int n, int k);
void func1(int n, int k)
{
    std::this_thread::sleep_for(std::chrono::seconds(k));
    printf("test for the thread of c++, %d\n", n);
}

int main() {

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

    //     Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
//     Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
//     Setup Dear ImGui style
    ImGui::StyleColorsDark();

	// Initializing mouse info
	lastMouseX = SCR_WIDTH / 2.0;
	lastMouseY = SCR_HEIGHT / 2.0;
	firstMouse = true;

    // Creating the density map
    int dim = 100;
    DensityMap grid(dim);

    // Add a sphere to the center of the grid
    //fanDemo(grid);
    //sphereDemo(grid);
    //realDemo(grid);
    //gainControl(grid, 0);
    //grid.setThreshold(0);

    // Creating the probe
    Probe probe("data/PROBE_CENTERED.stl");
    // Open the IMU file for reading
    //probe.openIMUFile("data/real_imu.txt");

	// Add all non-empty cells to the map
	grid.setThreshold(1);

    // multi-thread
    // thread1: read data from txt files, generate IMU file, and modify the grid.cell
    std::thread dataThread;
    dataThread = std::thread(realDemo, std::ref(grid), std::ref(dataUpdate));
    dataThread.detach();

    // thread2: add Gain control to the grid.cell after 30s automatically.
//    std::thread gainThread;
//    gainThread = std::thread(gainControl, std::ref(grid), 2, std::ref(dataUpdate));
//    gainThread.detach();

	// Main event loop

	renderLoop(window, probe, grid, windowTitle);

    // IMGUI Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	// GLFW cleanup
	glfwTerminate();
}



void renderLoop(GLFWwindow* window, Probe& probe, DensityMap& grid, std::string windowTitle)
{
    // Variables for measuring FPS
    int numFrames = 0;
    double lastFPSUpdate = 0;

//    float fGain = 0;  // for update each loop
//    unsigned seed; // for the random gain demo
//    seed = time(0);
//    srand(seed);

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

        //IMGUI setup
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // TGC in the loop for GUI control
//        float nGain = (rand()%6)/6.0;
//        gainControl(grid, -1 * fGain);
//        gainControl(grid, nGain);
//        fGain = nGain;
//        grid.updateVertexBuffers();

        if (dataUpdate)
        {
            probe.openIMUFile("data/real_imu.txt");
            dataUpdate = false;
        }
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
        //probe.draw(projection, view, rotationX, rotationY);

        // Draw the density map and the surrounding cube
        grid.draw(projection, view, model);

        float gain;
        float cutoff;
        float brightness;
        // render your IMGUI
        ImGui::Begin("GUI");
//        ImGui::Text("Brightness");
        ImGui::SliderFloat("Brightness", &brightness, 0.0f, 100.0f);
//        ImGui::Text("Gain");
        ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
//        ImGui::Text("Cutoff Value");
        ImGui::SliderFloat("Cutoff", &cutoff, 0.0f, 1.0f);
        ImGui::End();

        // Render dear imgui into screen
        ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mousePressed = true;
    }

    if (!ImGui::GetIO().WantCaptureMouse && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mousePressed = false;
    }
}





