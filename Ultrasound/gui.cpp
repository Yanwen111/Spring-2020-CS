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
    marker1 = glm::vec3(0.0f, 0.0f, 0.0f);
    marker2 = glm::vec3(0.0f, 10.0f, 0.0f);
    reset();
}

void GUI::drawGUI(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    setUp();
    isReset = false;
    drawWidgets(projection, view, model);
    if(setMarker){
        drawMarkers(projection, view, model);
    }
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
    threshold = 0;
    isReset = true;
    setMarker = false;
    marker1 = glm::vec3(0,0,0);
    marker2 = glm::vec3(0,5,0);
}

void GUI::drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    marker.draw(projection, view, model);

}

int GUI::getThreshold() {
    return threshold;
}

void GUI::drawWidgets(glm::mat4 projection, glm::mat4 view, glm::mat4 model){

    // render your IMGUI
    ImGui::Begin("GUI");
    ImGui::SliderFloat("Brightness", &brightness, 0.0f, 100.0f);
    ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
    ImGui::SliderInt("Threshold Cutoff", &threshold, 0, 255);
    ImGui::SliderInt("Zoom field of view", &zoom, 80, 10);
    ImGui::Checkbox("Set Marker", &setMarker);
    if (ImGui::Button("Reset"))
        reset();
    ImGui::End();

    ImGui::Begin("Statistics");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Time: %f", time);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Number of lines drawn: %d", numLines);
    ImGui::NewLine();
    ImGui::Text("Probe statistics");
    ImGui::Text("Quaternion values (xyzw):");
    ImGui::Indent(); ImGui::Text("%f %f %f %f", quat.x, quat.y, quat.z, quat.w);ImGui::Unindent();
    ImGui::Text("Euler Angle values:");
    ImGui::Indent(); ImGui::TextColored(ImColor(255, 255, 0, 255),"Roll,");
    ImGui::SameLine(0,40); ImGui::TextColored(ImColor(0, 255, 255, 255),"Pitch,");
    ImGui::SameLine(0,30); ImGui::TextColored(ImColor(255, 0, 255, 255),"Yaw");

    ImGui::TextColored(ImColor(255, 255, 0, 255), "%f", euler.x);ImGui::SameLine();
    ImGui::TextColored(ImColor(0, 255, 255, 255), "%f", euler.y);ImGui::SameLine();
    ImGui::TextColored(ImColor(255, 0, 255, 255), "%f", euler.z);

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
    numLines = num;
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