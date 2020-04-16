#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "gui.h"
#include "rotation.h"

GUI::GUI(GLFWwindow *window, const char* glsl_version, DensityMap* pointer){
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

    newLoad = false;
    loading = false;
    depth = 1500;
    gain = 1.0f;
    updateCoefficient = 1.0f;
    snap = false;
    snapThreshold = 70;

    scale = Scale();

    gridPointer = pointer;
}

void GUI::drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    setUp();
    isReset = false;
    modelWorld = model;
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
    snap = false;
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

    ImGui::Text("Load New File");
    ImGui::Indent();

        ImGui::Text("File name: data/");
        ImGui::SameLine();
        ImGui::PushItemWidth(300);
        ImGui::InputText(".txt", fileName, IM_ARRAYSIZE(fileName));
        ImGui::PopItemWidth();

        ImGui::Text("Select Probe Type");
        ImGui::Indent();
        ImGui::RadioButton("Submarine", &probeType, 0);
        ImGui::RadioButton("White Fin", &probeType, 1);
        ImGui::Unindent();

        ImGui::PushItemWidth(300);
        ImGui::SliderInt("Depth", &depth, 1, 2500);//CHECK?? IS THE DEPTH ALWAYS 2500???
        ImGui::SliderFloat("Gain", &gain, 0, 5);
        ImGui::SliderFloat("Update Weight", &updateCoefficient, 0, 1);
        ImGui::PopItemWidth();

        ImGui::PushID(1);
        if(loading) {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(207/360.0f, 0.4f, .4f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(207/360.0f, 0.4f, .4f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(207/360.0f, 0.4f, .4f));
            ImGui::Button("Load");
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImGui::Text("Loading...");
            newLoad = false;
        }
        else{
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(207/360.0f, 0.6f, .6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(207/360.0f, 0.8f, .8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(207/360.0f, 1.0f, 1.0f));
            if(ImGui::Button("Load"))
            {
                newLoad = true;
                loading = true;
            }
            else{
                newLoad = false;
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::PopID();

    ImGui::Unindent();

    ImGui::NewLine();

    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1/7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1/7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1/7.0f, 0.8f, 0.8f));
    if (ImGui::Button("Reset Settings"))
        reset();
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    ImGui::NewLine();

    ImGui::SliderFloat("Brightness", &brightness, -1.0f, 1.0f);
    ImGui::SliderFloat("Contrast", &contrast, 0.1f, 10.0f); //Goes to infinity, not including 0
    ImGui::SliderInt("Threshold Cutoff", &threshold, 0, 255);
    ImGui::SliderInt("Zoom field of view", &zoom, 80, 10);

    ImGui::NewLine();
    ImGui::Checkbox("Enable Snapping", &snap);
    ImGui::Indent();
    ImGui::PushItemWidth(300);
    ImGui::SliderInt("Snap Threshold", &snapThreshold, 0, 255);
    ImGui::PopItemWidth();
    ImGui::Unindent();

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

float GUI::getUpdateCoefficient(){
    return updateCoefficient;
}

void GUI::setUpdateCoefficient(float value){
    updateCoefficient = value;
}

int GUI::getDepth(){
    return depth;
}

std::string GUI::getFile(){
    std::string s(fileName);
    return ("data/" + s + ".txt");
}

void GUI::doneLoading(){
    loading = false;
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

int GUI::mouseClickedObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection) {
    //check if markers are on screen
    if(setMarker)
        return marker.checkMouseOnMarker(rayOrigin, rayDirection);

    return -1;
}

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

//get snap point in grid coordinate frame
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

        //check if val meets snap criteria
//        float shade = contrast * (val - 0.5) + 0.5 + brightness;
//        shade = shade * shade * shade * shade * shade;
//        if(shade < 0.003) shade = 0;
//        if(shade > 1.0) shade = 1.0;

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

void GUI::moveMarker(int numMarker,  glm::vec3 rayOrigin,  glm::vec3 rayDirection){
    if(numMarker == 1){
        glm::vec3 v0 = modelWorld*(glm::vec4(marker1x, marker1y, marker1z, 1)*10.0 - glm::vec4(5, 5, 5, 1)); //transform to world coordinates
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

double GUI::rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir){
    float denom = glm::dot(normal, rayDir);
    if (abs(denom) > 0.0001f) // your favorite epsilon
    {
        float t = glm::dot((point - rayOrig), normal) / denom;
        if (t >= 0) return t; // you might want to allow an epsilon here too
    }
    return -1;
}

bool GUI::loadNew(){
    return newLoad;
}
int GUI::getProbe(){
    return probeType;
}