#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "gui.h"
#include "rotation.h"

const ImVec4 blue = ImVec4(.196f,.40f,.663f, 100.0f);
const ImVec4 purple = ImVec4(.486f,.184f,.678f, 100.0f);
const ImVec4 orange = ImVec4(1.0f,.706f,.231f, 100.0f);

const int INPUT_TEXT_READ_ONLY = 16384;
const int INPUT_TEXT_PASSWORD = 32768;
const int INPUT_TEXT_CHARS_DECIMAL = 1;


const float GUI_WIDTH = 550;
const float GUI_HEIGHT = 700;

GUI::GUI(GLFWwindow *window, const char* glsl_version, DensityMap* pointer,
        void (*setZoom)(int),
        bool (*readData)(DensityMap&, std::string, float, int, bool&, std::string&),
         bool (*connectToProbe)(std::string, std::string, std::string, std::string,
                                bool, int, int, int, int, int, int,
                                std::string, int, std::string&, bool&
         )){
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    readDataMain = readData;
    setZoomMain = setZoom;
    connectToProbeMain = connectToProbe;

    //allocate 200 chars for the custom command
    screen2CustomCommand.reserve(200);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //Setup ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.196f, 0.4f, 0.663f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1558f,.3178f,.5265f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1558f,.3178f,.5265f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1558f,.3178f,.5265f, 1.00f);

    //Slider Colors
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(.027f,.18f,.38f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(.027f,.18f,.38f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(.831f,.882f,.949f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(.357f,.529f,.753f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(.643f,.584f,.835f, 1.00f);

    style.FrameRounding = 4;
    style.WindowPadding = ImVec2(15,15);



//    // Setup Marker class
//    marker = Marker();
//    // position of marker 1
//    glm::vec3 tmpPos = marker.getMarker1Pos();
//    marker1x = tmpPos.x;
//    marker1y = tmpPos.y;
//    marker1z = tmpPos.z;
//    // position of marker 2
//    tmpPos = marker.getMarker2Pos();
//    marker2x = tmpPos.x;
//    marker2y = tmpPos.y;
//    marker2z = tmpPos.z;

//    //position of the three scales
//    scaleX1 = scaleX2 = 1;
//    scaleY1 = 0;
//    scaleY2 = 1;
//    scaleZ1 = 0;
//    scaleZ2 = 1;
//
//    //the medium selected to set up velocity
//    mediumActive = 0;
//
//    reset();
//
//    //Whether the user clicked the load button
//    newLoad = false;
//    //If the data thread is loading a new file
//    loading = false;
//    // depth of the line data to display (0th cell to the depth cell)
//    depth = 1500;
//    // parameter for the time gain control
//    gain = 1.0f;
//    // weighting value (to control how DensityMap handles new data in the same cells)
//    updateCoefficient = 1.0f;
//    // Enable snapping when moving markers
//    snap = false;
//    // The threshold of values to snap to
//    snapThreshold = 70;
//

    //Set up the scale
    scale = Scale();

    //Pointer to the DensityMap grid object
    gridPointer = pointer;
}

//Draws the GUI on the screen and processes different user interactions to update values
void GUI::drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    setUp();
//    isReset = false;
//    modelWorld = model;
    drawWidgets(projection, view, model);
    interactionHandler();

//    //set up velocity
//    if(mediumActive == 0) velocity = 1102;
//    if(mediumActive == 1) velocity = 1538;
//    if(mediumActive == 2) velocity = atof(currVelocity);
//
//    if(setMarker){
//        drawMarkers(projection, view, model);
//    }
    drawScale(projection, view, model);
    render();
}

void GUI::render(){
    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void GUI::setUp(){
    //IMGUI setup
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::cleanUp(){
    // IMGUI Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Resets parameters to original default values
void GUI::reset(){
    dispDepth = 1500;
    dispGain = 0;
    dispWeight = 1;
    dispBrightness = 0.0f;
    dispContrast = 1.0f;
    dispCutoff = 1;
    dispZoom = 70;
}

//Draw the scales
void GUI::drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    scale.setMeasurements(dispFreq, dispVel, dispDepth);
    scale.draw(projection, view, model, glm::vec2(scaleXY, scaleXZ), glm::vec2(scaleYX, scaleYZ), glm::vec2(scaleZX, scaleZY));
}

////Draw the markers
//void GUI::drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
//    marker.setPositionMarker1(glm::vec3(marker1x,marker1y,marker1z));
//    marker.setPositionMarker2(glm::vec3(marker2x,marker2y,marker2z));
//    marker.draw(projection, view, model);
//}

void addText(const char* text, ImVec4 color=ImVec4(0,0,0,1.0f), float size=1.0f){
    ImGui::SetWindowFontScale(size);
    ImGui::TextColored(color, text);
    ImGui::SetWindowFontScale(1.0f);
}

void createToolTip(const char* text){
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(.937f,0.902f,0.961f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(350.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::PopStyleColor(2);
}

void purpleButton(const char* text, bool& pressed, float width=100, float height=50){
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.486f, .184f, .678f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.647f, 0.439f, 0.78f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    pressed = ImGui::Button(text, ImVec2(width, height));
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

void purpleButtonDisabled(const char* text, bool& pressed, float width=100, float height=50){
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    pressed = ImGui::Button(text, ImVec2(width, height));
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

void yellowButton(const char* text, bool& pressed){
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, .988f, .231f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f,	0.996f,	0.663f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    pressed = ImGui::Button(text);
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

void yellowButtonClicked(const char* text, bool& pressed){
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    pressed = ImGui::Button(text);
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

void drawOpenFrame(bool& pressedLoad, bool& pressedScan){
    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT / 2.0));
//    ImGui::SetNextWindowPos(ImVec2(500,300));

    ImGui::Begin("Ultrasonos");
    float startx = ImGui::GetWindowSize().x * 0.5f;
    ImGui::NextColumn();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::Indent(startx - 110);
    addText("Ultrasonos", ImColor(0,0,0, 255), 3);
    ImGui::Unindent(startx - 110);
    ImGui::NewLine();
    ImGui::Indent(startx - 170);
    addText("Click on one of the two options below to begin");
    ImGui::Unindent(startx - 170);
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::Indent(ImGui::GetWindowSize().x * 0.5f - 120);
    purpleButton("Load File", pressedLoad);
    ImGui::SameLine();
    ImGui::Indent(120);
    purpleButton("Scan", pressedScan);
    ImGui::End();
}

char currVelocity[10] = { 0 };
void displaySettings(bool isLoadData,
                     //display
                     int& depth, float& gain, float& weight,
                     float& brightness, float& contrast, int& cutoff, int& zoom,
                     bool& resetParametersPressed,
                     //speed of sound
                     int& mediumActive,
                     float velocity, float freq, std::string& inputVel,

                     //scale
                     float& scaleXY, float& scaleXZ, float& scaleYX, float& scaleYZ, float& scaleZX, float& scaleZY
        ) {
    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT));
    ImGui::Begin("Display Settings");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
    if(ImGui::CollapsingHeader("Display Parameters")){
        ImGui::NewLine();
        if (!isLoadData) {
            addText("Set Display Parameters");
            ImGui::Indent();
            addText("Depth");
            ImGui::Indent();
            ImGui::PushItemWidth(-1);
            ImGui::SliderInt("##depth", &depth, 1, 2500);
            createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
            ImGui::PopItemWidth();
            ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

            ImGui::Indent();
            addText("Gain");
            ImGui::Indent();
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##gain", &gain, 0, 5);
            createToolTip("Time gain compensation value.\n"
                          "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
            ImGui::PopItemWidth();
            ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

            ImGui::Indent();
            addText("Weight");
            ImGui::Indent();
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##weight", &weight, 0, 1);
            createToolTip("Weight to handle data in the same cell.\n\n"
                          "cell value = previous + new * weight");
            ImGui::PopItemWidth();
            ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();
        }
        ImGui::Indent();
        addText("Brightness");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##brightness", &brightness, -1, 1);
        createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
        ImGui::PopItemWidth();
        ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

        ImGui::Indent();
        addText("Contrast");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##contrast", &contrast, 0.1f, 10);
        createToolTip("Time gain compensation value.\n"
                      "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
        ImGui::PopItemWidth();
        ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

        ImGui::Indent();
        addText("Threshold Cutoff");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("##threshold", &cutoff, 0,255);
        createToolTip("Weight to handle data in the same cell.\n\n"
                      "cell value = previous + new * weight");
        ImGui::PopItemWidth();
        ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

        ImGui::Indent();
        addText("Zoom field of view");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("##zoom", &zoom,  80, 10);
        createToolTip("Weight to handle data in the same cell.\n\n"
                      "cell value = previous + new * weight");
        ImGui::PopItemWidth();
        ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

        purpleButton("Reset Parameters", resetParametersPressed, 130, 25);
        ImGui::NewLine();
    }
    if(ImGui::CollapsingHeader("Select Speed of Sound in Medium")){
        ImGui::NewLine();
        ImGui::Indent();
        ImGui::RadioButton("Silicone Gel", &mediumActive, 0);
        ImGui::RadioButton("Soft Tissue", &mediumActive, 1);
        ImGui::RadioButton("Input Speed: ", &mediumActive, 2);
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::InputText("m/s", currVelocity, IM_ARRAYSIZE(currVelocity));

        inputVel = currVelocity;

        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::NewLine();

        addText("Velocity through medium: "); ImGui::SameLine();
        addText((std::to_string(velocity) + " m/s").c_str(), purple);
        addText("Frequency:               "); ImGui::SameLine();
        addText((std::to_string(freq) + " MHz").c_str(), purple);
        ImGui::NewLine();
    }
    if(ImGui::CollapsingHeader("Marker Options")){
        ImGui::NewLine(); ImGui::Indent();
        addText("Select Marker Pair"); ImGui::SameLine();
        //Select marker pair
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD"};
        static const char* current_item = NULL;
        if (ImGui::BeginCombo("##markerPair", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1, 1, 1.00f));
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(items[n], is_selected))
                    current_item = items[n];
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::PopStyleColor();
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor();
        ImGui::Unindent(); ImGui::NewLine();
    }
    if(ImGui::CollapsingHeader("Scale Options")){
        ImGui::NewLine();
        addText("Scale shown in cm", blue);

        ImGui::Indent();
        addText("Scale X Location"); ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleXY", &scaleXY, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("Y     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleXZ", &scaleXZ, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("Z");
        ImGui::Unindent(); ImGui::Unindent();

        ImGui::Indent();
        addText("Scale X Location"); ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleYX", &scaleYX, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("X     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleYZ", &scaleYZ, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("Z");
        ImGui::Unindent(); ImGui::Unindent();

        ImGui::Indent();
        addText("Scale X Location"); ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleZX", &scaleZX, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("X     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("##scaleZY", &scaleZY, 0, 1);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        addText("Y");
        ImGui::Unindent(); ImGui::Unindent();

    }
    ImGui::PopStyleColor();
    ImGui::End();
}


//filterState = 0: no filter, 1 = submarine files, 2 = whitefin files
std::vector<std::string> getFileDirectories(int filterState = 0){
    std::vector<std::string> items;
    std::string path = "data/";
    for (const auto & entry : std::__fs::filesystem::directory_iterator(path))
        items.push_back(entry.path());
    return items;
}

//currState: 0 = no message, 1 = loading, 2 = success, 3 = error
void loadDataFromFile(
        std::string &file,
        int& depth, float& gain, float& weight,
        bool& load,
        int currState, std::string errorMessage
        ){ // loadDataFromFile(file, depth, gain, weight, error)
    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT));
    ImGui::Begin("Load Data from File");

    addText("Load Data from File", ImColor(0,0,0,255));
    ImGui::Indent();
    addText("Select File");
    ImGui::Indent();

    addText("Filter by Probe Type: "); ImGui::SameLine();
    bool submarineFilter = false;
    bool whiteFinFilter = false;
    static int filterType = 0;

    if(filterType == 0){
        yellowButton("Submarine", submarineFilter); ImGui::SameLine();
        yellowButton("White Fin", whiteFinFilter);

        if(submarineFilter) filterType = 1;
        if(whiteFinFilter) filterType = 2;
    }
    else if(filterType == 1){
        yellowButtonClicked("Submarine", submarineFilter); ImGui::SameLine();
        yellowButton("White Fin", whiteFinFilter);

        if(submarineFilter) filterType = 0;
        if(whiteFinFilter) filterType = 2;
    }
    else if(filterType == 2){
        yellowButton("Submarine", submarineFilter); ImGui::SameLine();
        yellowButtonClicked("White Fin", whiteFinFilter);

        if(submarineFilter) filterType = 1;
        if(whiteFinFilter) filterType = 0;
    }

    //Select File Directory
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));

    std::vector<std::string> items = getFileDirectories(filterType);
    static std::string current_item;
    if (ImGui::BeginCombo("##combo", current_item.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1, 1, 1.00f));
        for (auto & item : items)
        {
            const char* currItem = (char*)item.c_str();
            bool is_selected = (current_item == currItem); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(currItem, is_selected))
                current_item = currItem;
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    if (!current_item.empty())
        file.assign(current_item);

    ImGui::Unindent(); ImGui::Unindent(); ImGui::NewLine();

    addText("Set Display Parameters");
    ImGui::Indent();
    addText("Depth");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderInt("##depth", &depth, 1, 2500);
    createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
    ImGui::PopItemWidth();
    ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

    ImGui::Indent();
    addText("Gain");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("##gain", &gain, 0, 5);
    createToolTip("Time gain compensation value.\n"
                  "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
    ImGui::PopItemWidth();
    ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

    ImGui::Indent();
    addText("Weight");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("##weight", &weight, 0, 1);
    createToolTip("Weight to handle data in the same cell.\n\n"
                  "cell value = previous + new * weight");
    ImGui::PopItemWidth();
    ImGui::Unindent();ImGui::Unindent();ImGui::NewLine();

    ImGui::PopStyleColor();

    ImGui::NewLine();ImGui::NewLine();

    if(currState == 0){
        purpleButton("Load", load);
    }
    else if(currState == 1){
        //loading
        purpleButtonDisabled("Load", load);
        ImGui::SameLine();
        addText("Loading...", purple);
        //disable loadButton...
    }
    else if(currState == 2){
        //success!
        purpleButton("Load", load);
        ImGui::SameLine();
        addText((file + " sucessfully loaded").c_str(), blue);
    }
    else if(currState == 3){
        purpleButton("Load", load);
        ImGui::SameLine();
        addText("Error Message", orange);
    }

    ImGui::End();
}

void scanFromProbe(
        std::string* probeIP, std::string* probeUsername, std::string* probePassword, std::string* compIP,
        bool& isSubmarine, bool& isDefault,
        std::string* lxRangeMin, std::string* lxRangeMax, std::string* lxRes,
        std::string* servoRangeMin, std::string* servoRangeMax, std::string* servoRes,
        std::string* customCommand,
        bool& liveScan, bool& scanToFile, bool& sendCustom,
        int& currState,
        std::string output
        ){
    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT));
    ImGui::Begin("Scan From Probe");

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));

    addText("Enter addresses for the probe and computer to establish a connection.", blue);
    ImGui::NewLine();
    addText("Probe"); ImGui::Indent();

    addText("IP Address: ");ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##ipAddressProbe", (char*)probeIP->c_str(), probeIP->capacity()+1,
            (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
    ImGui::PopItemWidth();
    createToolTip("Probe IP Address Finding Instructions");

    addText("username:   "); ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##username", (char*)probeUsername->c_str(), probeUsername->capacity()+1,
                     (currState == 1) ? INPUT_TEXT_READ_ONLY : 0);
    ImGui::PopItemWidth();
    createToolTip("Default is set to 'root'");

    addText("password:   ");ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##password", (char*)probePassword->c_str(), probePassword->capacity()+1,
                     (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_PASSWORD);
    ImGui::PopItemWidth();
    createToolTip("Default is set to 'root'");

    ImGui::Unindent();
    ImGui::NewLine();
    addText("Computer"); ImGui::Indent();

    addText("IP Address: ");ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##ipAddressComp", (char*)compIP->c_str(), compIP->capacity()+1,
                     (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
    ImGui::PopItemWidth();
    createToolTip("Run the following commands on your terminal to find the IP address based on your operating system.\n\n"
                  "Windows:   ipconfig | findstr IPv4\n"
                  "Mac/Linux: ifconfig | grep inet");
    ImGui::Unindent();

    ImGui::NewLine();
    ImGui::NewLine();
    addText("Select Probe Type: "); ImGui::SameLine();

    bool submarine = false;
    bool whiteFin = false;
    if(currState != 1 && isSubmarine){
        yellowButtonClicked("Submarine", submarine); ImGui::SameLine();
        yellowButton("White Fin", whiteFin);

        if(whiteFin)
            isSubmarine = false;
    }
    else if(currState != 1) {
        yellowButton("Submarine", submarine); ImGui::SameLine();
        yellowButtonClicked("White Fin", whiteFin);

        if(submarine)
            isSubmarine = true;
    }

    ImGui::NewLine();

    bool defaultCon = false;
    bool advancedCon = true;
    if(currState != 1 && isDefault){
        yellowButtonClicked("   Default Connection   ", defaultCon); ImGui::SameLine();
        yellowButton("   Advanced Connection   ", advancedCon);

        if(advancedCon)
            isDefault = false;
    }
    else if(currState != 1) {
        yellowButton("   Default Connection   ", defaultCon); ImGui::SameLine();
        yellowButtonClicked("   Advanced Connection   ", advancedCon);

        if(defaultCon)
            isDefault = true;
    }
    ImGui::NewLine();
    ImGui::PopStyleColor();

    if(isDefault) {
        //White Fin Option
        if (!isSubmarine) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
            ImGui::PushTextWrapPos(GUI_WIDTH - 10);
            addText("Use default connection to make scans with the probe with the following parameter settings.", blue);
            ImGui::PopTextWrapPos();
            ImGui::NewLine();
            addText("Probe Scan Settings");

            ImGui::Indent();
            addText("Lx-16");

            ImGui::Indent();
            addText("Range:      ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##lxMin", (char*)lxRangeMin->c_str(), lxRangeMin->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("to");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##lxMax", (char*)lxRangeMax->c_str(), lxRangeMax->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("degrees");

            addText("Resolution: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##lxRes", (char*)lxRes->c_str(), lxRes->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("steps");

            //Servo
            ImGui::Unindent();
            addText("Servo");

            ImGui::Indent();
            addText("Range:      ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##servoMin", (char*)servoRangeMin->c_str(), servoRangeMin->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("to");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##servoMax", (char*)servoRangeMax->c_str(), servoRangeMax->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("degrees");

            addText("Resolution: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputText("##servoRes", (char*)servoRes->c_str(), servoRes->capacity()+1,
                             (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("steps");

            ImGui::Unindent();
            ImGui::Unindent();
            ImGui::PopStyleColor();
        }

        ImGui::NewLine();
        ImGui::PushTextWrapPos(GUI_WIDTH - 10);
        addText("Use the Live Scan option to  enter live mode and visualize scans. Use Scan to File option to scan and save into a .dat file.",
                purple);
        ImGui::PopTextWrapPos();

        ImGui::NewLine();
        purpleButton("Live Scan", liveScan);
        ImGui::SameLine();
        purpleButton("Scan to File", scanToFile);

    }
    else {
        addText("Enter the command to send to the probe below", blue);
        addText("*Only use if you're certain you know what you're doing.", orange);
        ImGui::NewLine();
        addText("Custom Command: ");

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
        ImGui::InputTextMultiline("##custom", (char*)customCommand->c_str(), customCommand->capacity()+1);
        ImGui::PopStyleColor();

        ImGui::NewLine();
        purpleButton("Live Scan", sendCustom);
    }

    //add user messages
    if(currState == 1)
        addText("Connecting to probe...", purple);
    if(currState == 2)
        addText("Successfully connected to probe", blue);
    if(currState == 3)
        addText("ERROR connecting to probe", orange);

    ImGui::NewLine();

    //connection output
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.00f));

    if(currState == 2)
        ImGui::SetNextTreeNodeOpen(true);

    if (ImGui::CollapsingHeader("Connection Output")) {
        ImGui::PopStyleColor();
        addText("Probe output: ");
        ImGui::Indent();
        addText(output.c_str());
    } else{
        ImGui::PopStyleColor();
    }

    ImGui::End();
}
//Draws the ImGui widgets on the screen
void GUI::drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    // render your IMGUI
    if(renderedScreen == 0)
        drawOpenFrame(screen0Load, screen0Scan);
    else if(renderedScreen == 1)
        loadDataFromFile(screen1File, dispDepth, dispGain, dispWeight, screen1Load, screen1CurrState, screen1Error);
    else if(renderedScreen == 2)
        scanFromProbe(&screen2ProbeIP, &screen2ProbeUsername, &screen2ProbePassword, &screen2CompIP, screen2IsSub, screen2IsDefault,
                &screen2LxMin, &screen2LxMax, &screen2LxRes, &screen2ServoMin, &screen2ServoMax, &screen2ServoRes,
                &screen2CustomCommand,
                screen2LiveScan, screen2ScanToFile, screen2SendCustom,
                screen2CurrState, screen2Output
                );

    if(isDataLoaded)
        displaySettings(
                isLoadFile, dispDepth, dispGain, dispWeight, dispBrightness, dispContrast, dispCutoff, dispZoom,
                dispReset,
                mediumActive, dispVel, dispFreq, inputVel,
                scaleXY, scaleXZ, scaleYX, scaleYZ, scaleZX, scaleZY
        );
}

void GUI::interactionHandler() {
    if(renderedScreen == 0){
        if(screen0Load == 1){
            renderedScreen = 1;
            isLoadFile = true;
        }
        else if(screen0Scan == 1){
            renderedScreen = 2;
            isLoadFile = false;
        }
    }
    if(renderedScreen == 1){
        if(screen1Load){
            //first time clicking load
            if(screen1CurrState == 0 || screen1CurrState == 2){
                //reload file
                gridPointer -> clear();
                screen1DataUpdate = false;
                isDataLoaded = false;

                //maybe do checks to make sure all fields populated?

                screen1CurrState = 1;

                //loadFile pointer function from main --> will only have 1 load file now with new data type
                bool noError = readDataMain(*gridPointer, screen1File, dispGain, dispDepth, screen1DataUpdate, screen1Error);
                gridPointer -> setUpdateCoefficient(screen1DataUpdate);

                if(!noError){
                    screen1CurrState = 3;
                }
            }
        }
        else{
            if(screen1DataUpdate) {
                screen1CurrState = 2;
                isDataLoaded = true;
            }
        }
    }
    if(renderedScreen == 2){
        try {
            if (screen2LiveScan && screen2CurrState != 1) {
                screen2Connected = false;
                screen2CurrState = 1;
                gridPointer->clear();
                isDataLoaded = false;
                if (screen2IsSub) {
                    bool noError = connectToProbeMain(screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                      screen2ProbePassword.c_str(), screen2CompIP.c_str(),
                                                      screen2IsSub, 0, 0, 0,
                                                      0, 0, 0,
                                                      "", 0, screen2Output, screen2Connected);
                    if (!noError) screen2CurrState = 3;
                } else {
                    bool noError = connectToProbeMain(screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                      screen2ProbePassword.c_str(),
                                                      screen2CompIP.c_str(),
                                                      screen2IsSub, std::stoi(screen2LxMin), std::stoi(screen2LxMax),
                                                      std::stoi(screen2LxRes),
                                                      std::stoi(screen2ServoMin), std::stoi(screen2ServoMax),
                                                      std::stoi(screen2ServoRes),
                                                      "", 0, screen2Output, screen2Connected);
                    if (!noError) screen2CurrState = 3;
                }
            }
            if (screen2ScanToFile && screen2CurrState != 1) {
                screen2Connected = false;
                screen2CurrState = 1;
                gridPointer->clear();
                isDataLoaded = false;
                if (screen2IsSub) {
                    bool noError = connectToProbeMain(screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                      screen2ProbePassword.c_str(), screen2CompIP.c_str(),
                                                      screen2IsSub, 0, 0, 0,
                                                      0, 0, 0,
                                                      "", 1, screen2Output, screen2Connected);
                    if (!noError) screen2CurrState = 3;
                } else {
                    bool noError = connectToProbeMain(screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                      screen2ProbePassword.c_str(),
                                                      screen2CompIP.c_str(),
                                                      screen2IsSub, std::stoi(screen2LxMin), std::stoi(screen2LxMax),
                                                      std::stoi(screen2LxRes),
                                                      std::stoi(screen2ServoMin), std::stoi(screen2ServoMax),
                                                      std::stoi(screen2ServoRes),
                                                      "", 1, screen2Output, screen2Connected);
                    if (!noError) screen2CurrState = 3;
                }
            }
            if (screen2SendCustom && screen2CurrState != 1) {
                screen2Connected = false;
                screen2CurrState = 1;
                gridPointer->clear();
                isDataLoaded = false;
                bool noError = connectToProbeMain(screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                  screen2ProbePassword.c_str(), screen2CompIP.c_str(),
                                                  screen2IsSub, 0, 0, 0,
                                                  0, 0, 0,
                                                  screen2CustomCommand.c_str(), 2, screen2Output, screen2Connected);
                if (!noError) screen2CurrState = 3;
            }
        }
        catch (...) {
            screen2CurrState = 3;
        }

        if(screen2Connected){
            screen2CurrState = 2;
            isDataLoaded = true;
        }
    }

    if(isDataLoaded) {
        //display parameters
        if(dispReset){
            reset();
            dispReset = false;
        }
        gridPointer -> setBrightness(dispBrightness);
        gridPointer -> setThreshold(dispCutoff);
        gridPointer -> setContrast(dispContrast);

        setZoomMain(dispZoom);

        //set speed of sound
        if(mediumActive == 0) dispVel = 1102;
        if(mediumActive == 1) dispVel = 1538;
        if(mediumActive == 2) dispVel = atof(inputVel.c_str());
    }
}

//void GUI::setNumLinesDrawn(int num){
//    if(num == -1)
//        numLines = 0;
//    else
//        numLines = num;
//}
//
//void GUI::setNumSamples(int num){
//    numSamples = num;
//}
//
//void GUI::setVoxels(int size){
//    voxels = size;
//}
//
//float GUI::getBrightness(){
//    return brightness;
//}
//
//float GUI::getGain(){
//    return gain;
//}
//
//float GUI::getUpdateCoefficient(){
//    return updateCoefficient;
//}
//
//void GUI::setUpdateCoefficient(float value){
//    updateCoefficient = value;
//}
//
//int GUI::getDepth(){
//    return depth;
//}
//
//std::string GUI::getFile(){
//    std::string s(fileName);
//    return ("data/" + s + ".txt");
//}
//
//void GUI::doneLoading(){
//    loading = false;
//}
//
//int GUI::getThreshold() {
//    return threshold;
//}
//
//void GUI::setBrightness(float value) {
//    brightness = value;
//}
//
//void GUI::setGain(float value) {
//    gain = value;
//}
//
//void GUI::setThreshold(int value) {
//    threshold = value;
//}
//
//void GUI::setContrast(float value){
//    contrast = value;
//}
//
//float GUI::getContrast(){
//    return contrast;
//}
//
//void GUI::setFileSize(double size){
//    fileSize = size;
//}
//
//void GUI::setTime(float currTime){
//    time = currTime;
//}
//
//void GUI::setQuaternion(glm::vec4 quatIn) {
//    quat = quatIn;
//}
//
//void GUI::setEulerAngles(glm::vec3 eulerIn) {
//    euler = eulerIn;
//}
//
//int GUI::getZoom(){
//    return zoom;
//}

/**
 * Returns whether the mouse ray intersects any specific GUI object on the screen.
 * @param rayOrigin The origin of the ray created by the mouse. Most likely the camera location
 * @param rayDirection The direction of the ray
 * @return which marker is intersected. -1 otherwise
 */
int GUI::mouseClickedObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection) {
    //check if markers are on screen
//    if(setMarker)
//        return marker.checkMouseOnMarker(rayOrigin, rayDirection);

    return -1;
}

/**
 * Finds the intersection of a ray with a cube and sets the starting t and ending t for the two intersections.
 * @param rayOriginGrid ray origin (e)
 * @param rayDirectionGrid ray direction (d)
 * @param tmin Satisfies the first point of intersection: e + t_min * d = P1
 * @param tmax Satisfies the second point of intersection: e + t_max * d = P2
 * @return true for intersection, false otherwise
 */
bool GUI::intersectGrid(glm::vec3 rayOriginGrid, glm::vec3 rayDirectionGrid, float& tmin, float& tmax) {
    //dimensions of box: -5, 5 for all axes
    float min = -5;
    float max = 5;

    tmin = (min - rayOriginGrid.x) / rayDirectionGrid.x;
    tmax = (max - rayOriginGrid.x) / rayDirectionGrid.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (min - rayOriginGrid.y) / rayDirectionGrid.y;
    float tymax = (max - rayOriginGrid.y) / rayDirectionGrid.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (min - rayOriginGrid.z) / rayDirectionGrid.z;
    float tzmax = (max - rayOriginGrid.z) / rayDirectionGrid.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    return true;
}

/**
 * Gets the point to snap to with the current ray and point
 * @param rayOrigin
 * @param currPoint
 * @return the new point to snap to
 */
glm::vec3 GUI::getSnapPoint(glm::vec3 rayOrigin, glm::vec3 currPoint){

    float tmin, tmax;

    glm::vec3 rayDirection = glm::normalize(currPoint - rayOrigin);

    //Transform to grid coordinates
    glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0,0,0,1));
    glm::vec3 rayOriginGrid = glm::transpose(rotation)*glm::vec4(rayOrigin.x, rayOrigin.y, rayOrigin.z, 1);
    glm::vec3 rayDirectionGrid = glm::transpose(rotation)*glm::vec4(rayDirection.x, rayDirection.y, rayDirection.z, 1);

    //Get intersection t values for ray with DensityMap grid in grid coordinates
    if(!intersectGrid(rayOriginGrid, rayDirectionGrid, tmin, tmax))
        return currPoint;

    //convert grid coordinates to grid cell values [0 - 1]
    glm::vec3 p0 = (rayOriginGrid + tmin*rayDirectionGrid + glm::vec3(5,5,5))/10.0;
    glm::vec3 p1 = (rayOriginGrid + tmax*rayDirectionGrid + glm::vec3(5,5,5))/10.0;

    //get point to snap to in grid cell values
    glm::vec3 destP = getSnapPointGrid(p0, p1, 100);

    //If nothing to snap to, then get original point
    if(destP.x == -1 && destP.y == -1 && destP.z == -1)
        return currPoint;

    //convert grid cell coordinates to grid coordinates
    destP = destP*10.0 - glm::vec3(5, 5, 5);
    //convert grid coordinates to world coordinates
    destP = modelWorld*glm::vec4(destP.x, destP.y, destP.z, 1);

    return destP;
}

/**
 * get snap point in grid coordinate frame
 * @param p1 First point of intersection
 * @param p2 Second point of intersection
 * @param numVals Number of values to get between the two points
 * @return The first point between p1 and p2 where the grid value is greater than the threshold
 */
glm::vec3 GUI::getSnapPointGrid(glm::vec3 p1, glm::vec3 p2, int numVals) {
    // x, y, and z coordinates of the current data point
    // Moves along the line defined by p1 and p2
    float x = p1.x;
    float y = p1.y;
    float z = p1.z;

    // Direction of the line defined by p1 and p2
    float dx = (p2.x - p1.x) / numVals;
    float dy = (p2.y - p1.y) / numVals;
    float dz = (p2.z - p1.z) / numVals;

    for (int i = 0; i < numVals; i++) {
        auto val = (float)gridPointer->readCellInterpolated(x, y, z);

        if(val > snapThreshold)
            return glm::vec3(x, y, z);

        // Move x, y, and z along the line
        x += dx;
        y += dy;
        z += dz;
    }

    //if none meets criteria, return null point
    return glm::vec3(-1, -1, -1);
}

/**
 * Move the specified marker to the cursor position.
 * @param numMarker Which marker the cursor is on
 * @param rayOrigin origin of the ray
 * @param rayDirection direction of the ray
 */
void GUI::moveMarker(int numMarker,  glm::vec3 rayOrigin,  glm::vec3 rayDirection){
    if(numMarker == 1){
        //transform to world coordinates
        glm::vec3 v0 = modelWorld*(glm::vec4(marker1x, marker1y, marker1z, 1)*10.0 - glm::vec4(5, 5, 5, 1));
        glm::vec4 normal = glm::vec4(0,0,1,0);
        double t = rayPlaneIntersect(normal, v0, rayOrigin, rayDirection);

        //Find the intersection point on the plane
        glm::vec3 P = rayOrigin + t*rayDirection;

        //Find the snapping point on the plane
        if(snap)
            P = getSnapPoint(rayOrigin, P);

        //Transform back to marker coordinates
        glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0,0,0,1));
        P = glm::transpose(rotation)*glm::vec4(P.x, P.y, P.z, 1);

        P = (P + glm::vec3(5, 5, 5)) / 10.0;

        if(P.x > 1) P.x = 1;
        if(P.x < 0) P.x = 0;
        if(P.y > 1) P.y = 1;
        if(P.y < 0) P.y = 0;
        if(P.z > 1) P.z = 1;
        if(P.z < 0) P.z = 0;

        marker1x = P.x;
        marker1y = P.y;
        marker1z = P.z;
    }
    if(numMarker == 2){
        glm::vec3 v0 = modelWorld*(glm::vec4(marker2x, marker2y, marker2z, 1)*10.0 - glm::vec4(5, 5, 5, 1)); //transform to world coordinates
        glm::vec4 normal = glm::vec4(0,0,1,0);
        double t = rayPlaneIntersect(normal, v0, rayOrigin, rayDirection);

        //Find the intersection point on the plane
        glm::vec3 P = rayOrigin + t*rayDirection;

        //Find the snapping point on the plane
        if(snap)
            P = getSnapPoint(rayOrigin, P);

        //Transform back to marker coordinates
        glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0,0,0,1));
        P = glm::transpose(rotation)*glm::vec4(P.x, P.y, P.z, 1);

        P = (P + glm::vec3(5, 5, 5)) / 10.0;

        if(P.x > 1) P.x = 1;
        if(P.x < 0) P.x = 0;
        if(P.y > 1) P.y = 1;
        if(P.y < 0) P.y = 0;
        if(P.z > 1) P.z = 1;
        if(P.z < 0) P.z = 0;

        marker2x = P.x;
        marker2y = P.y;
        marker2z = P.z;
    }
}

/**
 * Calculates the value for t in the intersection between a plane and a ray
 * @param normal normal of the plane
 * @param point point on the plane
 * @param rayOrig ray origin (e)
 * @param rayDir ray direction (d)
 * @return the t value that satisfied e + td = P, where P is the intersection of the ray with the plane. -1 if no intersection
 */
double GUI::rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir){
    float denom = glm::dot(normal, rayDir);
    if (abs(denom) > 0.0001f) // your favorite epsilon
    {
        float t = glm::dot((point - rayOrig), normal) / denom;
        if (t >= 0) return t; // you might want to allow an epsilon here too
    }
    return -1;
}

//bool GUI::loadNew(){
//    return newLoad;
//}
//int GUI::getProbe(){
//    return probeType;
//}