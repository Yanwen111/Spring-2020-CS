#include <iostream>
#include <imgui.h>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "gui.h"
#include "rotation.h"

GUI::GUI(GLFWwindow *window, const char* glsl_version){

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    marker = Marker();
    glm::vec3 tmpPos = marker.getMarker1Pos();
    marker1x = tmpPos.x;
    marker1y = tmpPos.y;
    marker1z = tmpPos.z;
    tmpPos = marker.getMarker2Pos();
    marker2x = tmpPos.x;
    marker2y = tmpPos.y;
    marker2z = tmpPos.z;

    scaleX1 = scaleX2 = 1;
    scaleY1 = 0;
    scaleY2 = 1;
    scaleZ1 = 0;
    scaleZ2 = 1;

    mediumActive = 0;

    reset();

    scale = Scale();
}

void GUI::drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    setUp();
    isReset = false;
    drawWidgets(projection, view, model);

    //set up velocity
    if(mediumActive == 0) velocity = 1102;
    if(mediumActive == 1) velocity = 1538;
    if(mediumActive == 2) velocity = atof(currVelocity);

    if(setMarker){
        drawMarkers(projection, view, model);
    }
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

void GUI::reset(){
    brightness = 0.0f;
    gain = 0.0f;
    zoom = 70;
    contrast = 1.0f;
    threshold = 1;
    isReset = true;
    setMarker = false;
    velocity = 1102;
    frequency = 15.6;
}

void GUI::drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    scale.setMeasurements(frequency, velocity, numSamples);
    scale.draw(projection, view, model, glm::vec2(scaleX1, scaleX2), glm::vec2(scaleY1, scaleY2), glm::vec2(scaleZ1, scaleZ2));
}

void GUI::drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    marker.setPositionMarker1(glm::vec3(marker1x,marker1y,marker1z));
    marker.setPositionMarker2(glm::vec3(marker2x,marker2y,marker2z));
    marker.draw(projection, view, model);
}

void GUI::drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    // render your IMGUI
    ImGui::Begin("GUI");

    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1/7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1/7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1/7.0f, 0.8f, 0.8f));
    if (ImGui::Button("Reset"))
        reset();

    ImGui::PopStyleColor(3);
    ImGui::PopID();

    ImGui::NewLine();

    ImGui::SliderFloat("Brightness", &brightness, -1.0f, 1.0f);
//    ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
    ImGui::SliderFloat("Contrast", &contrast, 0.1f, 10.0f); //Goes to infinity, not including 0
    ImGui::SliderInt("Threshold Cutoff", &threshold, 0, 255);
    ImGui::SliderInt("Zoom field of view", &zoom, 80, 10);

    ImGui::Checkbox("Set Marker", &setMarker);
    if(setMarker) {
        ImGui::Text("Distance between markers: ");
        ImGui::SameLine();
        ImGui::TextColored(ImColor(255, 255, 50, 255), "%f cm", marker.getDistance(frequency, velocity, numSamples));

        ImGui::Text("Marker 1 Movement");
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("X1", &marker1x, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Y1", &marker1y, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Z1", &marker1z, 0.0f, 1.0f);
        ImGui::PopItemWidth();

        ImGui::Text("Marker 2 Movement");
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("X2", &marker2x, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Y2", &marker2y, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Z2", &marker2z, 0.0f, 1.0f);
        ImGui::PopItemWidth();
    }

    ImGui::NewLine();
    ImGui::Text("Scale shown in cm");
    if (ImGui::CollapsingHeader("Scale Options")) {
        ImGui::Text("Scale X Location");
        ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("X scale: Y", &scaleX1, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("X scale: Z", &scaleX2, 0.0f, 1.0f);
        ImGui::PopItemWidth();

        ImGui::Unindent();
        ImGui::Text("Scale Y Location");
        ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("Y scale: X", &scaleY1, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Y scale: Z", &scaleY2, 0.0f, 1.0f);
        ImGui::PopItemWidth();

        ImGui::Unindent();
        ImGui::Text("Scale Z Location");
        ImGui::Indent();
        ImGui::PushItemWidth(80);
        ImGui::SliderFloat("Z scale: X", &scaleZ1, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("Z scale: Y", &scaleZ2, 0.0f, 1.0f);
        ImGui::PopItemWidth();
    }
    ImGui::End();

    ImGui::Begin("Statistics");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

    int hr = time / 3600;
    int min = (time - 3600 * hr) / 60.0;
    int sec = (time - 3600 * hr - 60 * min);
    int milli = (time - 3600*hr - 60*min - sec)*1000;
    ImGui::Text("Time: %d:%d:%d:%d", hr, min, sec, milli);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//    ImGui::Text("File size: %f", fileSize);
    ImGui::NewLine();

    ImGui::Text("Number of lines read in file: ");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 255, 50, 255), "%d", numLines);
    ImGui::Text("# Voxels: ");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 255, 50, 255), "%d x %d x %d", voxels, voxels, voxels);
    ImGui::Text("Number of samples per line: ");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 255, 50, 255), "%d", numSamples);
    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Select Speed of Sound in Medium: ")) {
        ImGui::Indent();
        ImGui::RadioButton("Silicone Gel", &mediumActive, 0);
        ImGui::RadioButton("Soft Tissue", &mediumActive, 1);
        ImGui::RadioButton("Input Speed: ", &mediumActive, 2);
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::InputText("m/s", currVelocity, IM_ARRAYSIZE(currVelocity));
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::NewLine();
    }

    ImGui::Text("Velocity used for calculation: ");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 255, 50, 255), "%f m/s", velocity);
    ImGui::Text("Frequency: ");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 255, 50, 255), "%f MHz", frequency);
    ImGui::NewLine();


    if (ImGui::CollapsingHeader("Probe Statistics")) {
        ImGui::Text("Quaternion values (xyzw):");
        ImGui::Indent();
        ImGui::Text("%f %f %f %f", quat.x, quat.y, quat.z, quat.w);
        ImGui::Unindent();
        ImGui::Text("Euler Angle values:");
        ImGui::Indent();
        ImGui::TextColored(ImColor(255, 255, 0, 255), "Roll,");
        ImGui::SameLine(0, 40);
        ImGui::TextColored(ImColor(0, 255, 255, 255), "Pitch,");
        ImGui::SameLine(0, 30);
        ImGui::TextColored(ImColor(255, 0, 255, 255), "Yaw");

        ImGui::TextColored(ImColor(255, 255, 0, 255), "%f", euler.x);
        ImGui::SameLine();
        ImGui::TextColored(ImColor(0, 255, 255, 255), "%f", euler.y);
        ImGui::SameLine();
        ImGui::TextColored(ImColor(255, 0, 255, 255), "%f", euler.z);
    }
    //Create reference frame
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    float cx = 60.0f;
    float cy = 80.0f;
    ImVec2 origin = ImVec2(cursor_pos.x + cx, cursor_pos.y + cy);

    glm::vec4 roll = 50.f*glm::vec4(1,0,0,1);
    glm::vec4 pitch = 50.f*glm::vec4(0,0,1,1);
    glm::vec4 yaw = 50.f*glm::vec4(0,-1,0,1);

    roll = view * model * roll;
    pitch = view * model * pitch;
    yaw = view * model * yaw;

    auto* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine(origin, ImVec2(origin.x + roll.x, origin.y - roll.y), ImColor(255, 255, 0, 255));
    draw_list->AddLine(origin, ImVec2(origin.x + pitch.x, origin.y - pitch.y), ImColor(0, 255, 255, 255));
    draw_list->AddLine(origin, ImVec2(origin.x + yaw.x,origin.y - yaw.y), ImColor(255, 0, 255, 255));

    //Circles to mark end of positive coordinates
    draw_list->AddCircleFilled(ImVec2(origin.x + roll.x, origin.y - roll.y), 5,ImColor(255, 255, 0, 255) );
    draw_list->AddCircleFilled(ImVec2(origin.x + pitch.x, origin.y - pitch.y), 5, ImColor(0, 255, 255, 255));
    draw_list->AddCircleFilled(ImVec2(origin.x + yaw.x,origin.y - yaw.y), 5, ImColor(255, 0, 255, 255));

    //Circles for negative coordinates
    draw_list->AddCircleFilled(ImVec2(origin.x - roll.x, origin.y + roll.y), 5,ImColor(255, 255, 0, 100) );
    draw_list->AddCircleFilled(ImVec2(origin.x - pitch.x, origin.y + pitch.y), 5, ImColor(0, 255, 255, 100));
    draw_list->AddCircleFilled(ImVec2(origin.x - yaw.x,origin.y + yaw.y), 5, ImColor(255, 0, 255, 100));

    //Add text for roll, pitch, yaw
    if(origin.x < roll.x)
        draw_list->AddText(ImVec2(origin.x + 0.7f*roll.x, origin.y - 1.3f*roll.y), ImColor(255, 255, 0, 255), "Roll");
    else
        draw_list->AddText(ImVec2(origin.x + 1.3f*roll.x, origin.y - 1.3f*roll.y), ImColor(255, 255, 0, 255), "Roll");
    if(origin.x < pitch.x)
        draw_list->AddText(ImVec2(origin.x + 0.7f*pitch.x, origin.y - 1.3f*pitch.y), ImColor(0, 255, 255, 255), "Pitch");
    else
        draw_list->AddText(ImVec2(origin.x + 1.3f*pitch.x, origin.y - 1.3f*pitch.y), ImColor(0, 255, 255, 255), "Pitch");
    if(origin.x < yaw.x)
        draw_list->AddText(ImVec2(origin.x + 0.7f*yaw.x,origin.y - 1.3f*yaw.y), ImColor(255, 0, 255, 255), "Yaw");
    else
        draw_list->AddText(ImVec2(origin.x + 1.3f*yaw.x,origin.y - 1.3f*yaw.y), ImColor(255, 0, 255, 255), "Yaw");

    ImGui::End();
}

void GUI::setNumLinesDrawn(int num){
    if(num == -1)
        numLines = 0;
    else
        numLines = num;
}

void GUI::setNumSamples(int num){
    numSamples = num;
}

void GUI::setVoxels(int size){
    voxels = size;
}

//ImGui::SliderFloat("Brightness", &brightness, 0.0f, 100.0f);
//ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
//ImGui::SliderInt("Threshold Cutoff", &threshold, 0, 255);
float GUI::getBrightness(){
    return brightness;
}

float GUI::getGain(){
    return gain;
}

int GUI::getThreshold() {
    return threshold;
}

void GUI::setBrightness(float value) {
    brightness = value;
}

void GUI::setGain(float value) {
    gain = value;
}

void GUI::setThreshold(int value) {
    threshold = value;
}

void GUI::setContrast(float value){
    contrast = value;
}

float GUI::getContrast(){
    return contrast;
}

void GUI::setFileSize(double size){
    fileSize = size;
}

void GUI::setTime(float currTime){
    time = currTime;
}

void GUI::setQuaternion(glm::vec4 quatIn) {
    quat = quatIn;
}

void GUI::setEulerAngles(glm::vec3 eulerIn) {
    euler = eulerIn;
}

int GUI::getZoom(){
    return zoom;
}