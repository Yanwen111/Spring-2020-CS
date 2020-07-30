#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "gui.h"
#include "rotation.h"
#include <ft2build.h>
#include FT_FREETYPE_H

const ImVec4 blue = ImVec4(.196f, .40f, .663f, 100.0f);
const ImVec4 purple = ImVec4(.486f, .184f, .678f, 100.0f);
const ImVec4 orange = ImVec4(1.0f, .706f, .231f, 100.0f);

const int INPUT_TEXT_READ_ONLY = 16384;
const int INPUT_TEXT_PASSWORD = 32768;
const int INPUT_TEXT_CHARS_DECIMAL = 1;

const int FONT_SIZE = 18;

const std::vector<glm::vec3> markerColors = {glm::vec3(1.0f, 0.0f, 0.8f), glm::vec3(1.0f, 0.8f, 0.0f),
                                             glm::vec3(0.0f, 1.0f, 0.8f)};

bool isDrawingBox = false;
bool drawObjectMode = false;

GUI::GUI(GLFWwindow *window, const char *glsl_version, DensityMap *pointer,
         void (*setZoom)(int),
         bool (*readData)(DensityMap &, std::string, float, int, bool &, std::string &, int &, bool &),
         bool (*connectToProbe)(DensityMap &, std::string, std::string, std::string, std::string,
                                bool, int, int, int, int, int, int,
                                std::string, int, std::string &, bool &, bool &, std::string &),
         void (*setDepth)(int),
         void (*setGain)(float),
         bool (*saveFile)(bool, bool&, std::string&, bool),
         glm::mat4 cameraToWorld_in,
         bool (*applyFilters_in)(DensityMap& grid, std::string file, float gain, int depth, bool& dataUpdate, std::vector<double> filterList)
) {
    glfwGetWindowSize(window, &width, &height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    readDataMain = readData;
    setZoomMain = setZoom;
    connectToProbeMain = connectToProbe;
    setDepthMain = setDepth;
    setGainMain = setGain;
    saveFileMain = saveFile;
    applyFiltersMain = applyFilters_in;

    cameraToWorld = cameraToWorld_in;

    //allocate 200 chars for the custom command
    screen2CustomCommand.reserve(200);

    filePath = boost::filesystem::current_path();
    std::cout << "########### Current path is : " << filePath << " ##########" << std::endl;

    // Setup FreeType Fonts
    setUpFont();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //Setup ImGui Style
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.196f, 0.4f, 0.663f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1558f, .3178f, .5265f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1558f, .3178f, .5265f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1558f, .3178f, .5265f, 1.00f);

    //Slider Colors
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(.027f, .18f, .38f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(.027f, .18f, .38f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(.831f, .882f, .949f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(.357f, .529f, .753f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(.643f, .584f, .835f, 1.00f);

    style.FrameRounding = 4;
    style.WindowPadding = ImVec2(15, 15);

    //Set up the scale
    scale = Scale();

    //Set up the probe
    probe = Probe();

//    myTexts.push_back( Text("TEST TEXT HI\nTHIS IS LINE ONE\nTHIS IS LINE TWO", width, height));
//    myTexts.push_back( Text("THIS IS ANOTHER TEXT", width, height));

    //Pointer to the DensityMap grid object
    gridPointer = pointer;

    //set all parameter values
    reset();

    //LOAD CONFIG FILE, IF EXISTS
    loadConfig();

    dispVel = 1102;

//    myObj = MeasureObject(gridPointer);
    myObj = MeasureObject(gridPointer);
}

void GUI::loadConfig() {
    std::string stemp;
    int number = 0;
    std::string config_name = "config_file/ssh_config_" + std::to_string(number) + ".txt";

    std::ifstream filein(config_name);

    if(filein) {
        char *temp = new char[30];

        filein.getline(temp, 100);
        screen2ProbeIP = temp;
        screen2ProbeIP = screen2ProbeIP.substr(12).c_str();

        filein.getline(temp, 100);
        screen2ProbeUsername = temp;
        screen2ProbeUsername = screen2ProbeUsername.substr(11).c_str();

        filein.getline(temp, 100);
        screen2ProbePassword = temp;
        screen2ProbePassword = screen2ProbePassword.substr(10).c_str();
    }

    filein.close();
    printf("Config file No.%d has been loaded!\n", number);
}

void GUI::setUpFont() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    std::string fontPath = filePath.string() + "/config_file/fonts/open-sans/OpenSans-Regular.ttf";
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    // Load first 128 characters of the ASCII character set
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //setup Shader for text
    std::string vmarker =
            "#version 330 core\n"
            "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
            "out vec2 TexCoords;\n"
            "\n"
            "uniform mat4 projection;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
            "    TexCoords = vertex.zw;\n"
            "}";

    std::string fmarker =
            "#version 330 core\n"
            "in vec2 TexCoords;\n"
            "out vec4 color;\n"
            "\n"
            "uniform sampler2D text;\n"
            "uniform vec3 textColor;\n"
            "\n"
            "void main()\n"
            "{    \n"
            "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
            "    color = vec4(textColor, 1.0) * sampled;\n"
            "}";

    textShader = Shader(vmarker.c_str(), fmarker.c_str(), false);
}

// render line of text
// -------------------
void GUI::RenderText(std::string text, float x, float y, float scaleIn, glm::vec3 color) {
    // activate corresponding render state
    textShader.use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    textShader.setMat4("projection", projection);
//    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
    textShader.setVec3("textColor", color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scaleIn;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scaleIn;

        float w = ch.Size.x * scaleIn;
        float h = ch.Size.y * scaleIn;
        // update VBO for each character
        float vertices[6][4] = {
                {xpos, ypos + h,     0.0f, 0.0f},
                {xpos, ypos,         0.0f, 1.0f},
                {xpos + w, ypos,     1.0f, 1.0f},

                {xpos, ypos + h,     0.0f, 0.0f},
                {xpos + w, ypos,     1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices),
                        vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) *
             scaleIn; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

//Draws the GUI on the screen and processes different user interactions to update values
void GUI::drawGUI(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY) {
    setUp();
    drawWidgets(projection, view);
    interactionHandler();

    glm::mat4 cubeRotation = glm::mat4(1.0f);
    cubeRotation = glm::rotate(cubeRotation, rotationY, glm::vec3(0, 1, 0));
    cubeRotation = glm::rotate(cubeRotation, rotationX,
                               glm::rotate(glm::vec3(1, 0, 0), rotationY, glm::vec3(0, -1, 0)));
    drawScale(projection, view, cubeRotation);
    drawMarkers(projection, view, cubeRotation);

    drawTexts(projection, view, cubeRotation);

    drawProbe(projection, view, rotationX, rotationY);

    drawObj(projection, view, cubeRotation);

    modelWorld = cubeRotation;

//    RenderText("This is sample text", 883.402344, 556, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

    render();
}

void GUI::render() {
    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::setUp() {
    //IMGUI setup
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::cleanUp() {
    //Remove all the temporary files created before closing program
    saveFileMain(false, screen2ErrorSaveFile, screen2ErrorMessage, true);

    // IMGUI Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Resets parameters to original default values
void GUI::reset() {
    dispWeight = 1;
    dispBrightness = 0.0f;
    dispContrast = 1.0f;
    dispCutoff = 1;
    dispZoom = 70;
}

//Draw the scales
void GUI::drawScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    scale.setMeasurements(dispFreq, dispVel, dispDepth);
    scale.draw(projection, view, model, glm::vec2(scaleXY, scaleXZ), glm::vec2(scaleYX, scaleYZ),
               glm::vec2(scaleZX, scaleZY));
}

//Draw the measure object
void GUI::drawObj(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    if(drawObjectMode)
        myObj.draw(projection, view, model);
}

//Draw the markers
void GUI::drawMarkers(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    for (auto &marker : markers) {
        if (!marker.getHidden())
            marker.draw(projection, view, model);
    }

    if (intersectedMarker != nullptr && showMarkerDistance) {
        RenderText("Marker " + std::to_string(intersectedMarker->getNumber()), markerXPos + FONT_SIZE, markerYPos, 1.0,
                   intersectedMarker->getColor());
        RenderText(std::to_string(intersectedMarker->getDistance(dispFreq, dispVel, dispDepth)) + " mm",
                   markerXPos + FONT_SIZE, markerYPos - FONT_SIZE, 1.0, intersectedMarker->getColor());
    }
}

//Draw the Texts
void GUI::drawTexts(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    for (auto &txt : myTexts) {
            txt.draw(projection, view, model);
    }

}

//############################## IMGUI Helper Functions ########################################################################

/**
 * Adds the specified text into the ImGUI Panel
 * @param text text to add on the panel
 * @param color color of the text. Default is black.
 * @param size Font size of the text of 18 scaled by this input. Default is 1.0 which is 18 font.
 */
void addText(const char *text, ImVec4 color = ImVec4(0, 0, 0, 1.0f), float size = 1.0f) {
    ImGui::SetWindowFontScale(size);
    ImGui::TextColored(color, text);
    ImGui::SetWindowFontScale(1.0f);
}

/**
 * Creates the hover tool tip with the following text.
 * @param text Text to display in the tool tip.
 */
void createToolTip(const char *text) {
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(.937f, 0.902f, 0.961f, 1.00f));
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

/**
 * Purple button component.
 * @param text Text to display in the button
 * @param pressed set to true if the button is pressed. False otherwise.
 * @param width default is 100px.
 * @param height default is 50 px.
 */
void purpleButton(const char *text, bool &pressed, float width = 100, float height = 50) {
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.486f, .184f, .678f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.647f, 0.439f, 0.78f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    pressed = ImGui::Button(text, ImVec2(width, height));
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

/**
 * Purple button component that is disabled.
 * @param text Text to display in the button
 * @param pressed set to true if the button is pressed. False otherwise.
 * @param width default is 100px.
 * @param height default is 50 px.
 */
void purpleButtonDisabled(const char *text, bool &pressed, float width = 100, float height = 50) {
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.243f, 0.02f, 0.388f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    pressed = ImGui::Button(text, ImVec2(width, height));
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

/**
 * Yellow Button component. Width is variable to input text length.
 * @param text text to display.
 * @param pressed set to true is button is pressed. False otherwise.
 */
void yellowButton(const char *text, bool &pressed) {
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, .988f, .231f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.996f, 0.663f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    pressed = ImGui::Button(text);
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

/**
 * Pressed Yellow Button component. Width is variable to input text length.
 * @param text text to display.
 * @param pressed set to true is button is pressed. False otherwise.
 */
void yellowButtonClicked(const char *text, bool &pressed) {
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.804f, .796f, 0.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    pressed = ImGui::Button(text);
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

/**
 * Draws the opening GUI panel screen of the software.
 * @param pressedLoad Set to true if the user clicks on load file button.
 * @param pressedScan Set to true if user clicks on scan button.
 * @param width Width of the GUI panel
 * @param height Height of the GUI panel
 */
void drawOpenFrame(bool &pressedLoad, bool &pressedScan, int width, int height) {
    ImGui::SetNextWindowSize(ImVec2(500,350));
    ImGui::SetNextWindowPos(ImVec2(width/2 - 250, height/2 - 175));

    ImGui::Begin("Ultrasonos");
    float startx = ImGui::GetWindowSize().x * 0.5f;
    ImGui::NextColumn();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::Indent(startx - 110);
    addText("Ultrasonos", ImColor(0, 0, 0, 255), 3);
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

char currVelocity[10] = {0};
/**
 * Method that displays the display settings panel after an image is loaded.
 *
 * Params used for display settings
 * @param depth pointer for the depth
 * @param gain pointer for the gain
 * @param weight pointer for the update weight value
 * @param brightness pointer for the brightness
 * @param contrast pointer for the contrast
 * @param cutoff pointer for the threshold cutoff
 * @param zoom pointer for the zoom field of view
 * @param resetParametersPressed set to true if user clicks on reset parameters
 *
 * Params used for select speed of sound
 * @param mediumActive int for the medium active selected by the user. 0 = silicone gel, 1 = soft tissue, 2 = custom input
 * @param velocity velocity for the medium selected
 * @param freq frequency for the medium selected
 * @param inputVel pointer for the custom velocity input
 *
 * Params used for the scale options
 * @param scaleXY Y coordinate of horizontal scale
 * @param scaleXZ Z coordinate of horizontal scale
 * @param scaleYX X coordinate of vertical scale
 * @param scaleYZ Z coordinate of vertical scale
 * @param scaleZX X coordinate of Z-axis scale
 * @param scaleZY Y coordinate of Z-axis scale
 * @param scale pointer to the scale object
 *
 * Params used for the marker options
 * @param markerList list containing the marker pairs
 *
 * Params used for the text options
 * @param textList list containing the text pairs
 *
 * Params used for the filter options
 * @param filterList list of the filters
 * @param applyFilter set to true if user clicks apply filters
 * @param currState int to indicate the state of applying filters. 0 = nothing, 1 = loading, 2 = successfully applied filters, 3 = error
 *
 * Params for snapping markers
 * @param enableSnap Set to true if snap is enabled.
 * @param snapThresholdIn Threshold value for snap
 *
 * Param for fit to object
 * @param myObj MeasureObject pointer to fit objects
 *
 * @param scr_width Screen Width
 * @param scr_height Screen Height
 */
void displaySettings(
                     int &depth, float &gain, float &weight,
                     float &brightness, float &contrast, int &cutoff, int &zoom,
                     bool &resetParametersPressed,
                     int &mediumActive,
                     float velocity, float freq, std::string &inputVel,
                     float &scaleXY, float &scaleXZ, float &scaleYX, float &scaleYZ, float &scaleZX, float &scaleZY,
                     Scale& scale,
                     std::vector<Marker> &markerList,
                    std::vector<Text> &textList,
                    std::vector<double> &filterList,
                     bool &applyFilter,
                     int &currState,
                     bool &enableSnap, int &snapThresholdIn,
                    MeasureObject &myObj,
                    int scr_width, int scr_height
) {
    ImGui::Begin("Display Settings");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
    if (ImGui::CollapsingHeader("Display Parameters")) {
        ImGui::NewLine();
        ImGui::Indent();
        addText("Brightness");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##brightness", &brightness, -1, 1);
        createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();
        ImGui::NewLine();

        ImGui::Indent();
        addText("Contrast");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##contrast", &contrast, 0.1f, 10);
        createToolTip("Time gain compensation value.\n"
                      "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();
        ImGui::NewLine();

        ImGui::Indent();
        addText("Threshold Cutoff");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("##threshold", &cutoff, 0, 255);
        createToolTip("Weight to handle data in the same cell.\n\n"
                      "cell value = previous + new * weight");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();
        ImGui::NewLine();

        ImGui::Indent();
        addText("Zoom field of view");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("##zoom", &zoom, 80, 10);
        createToolTip("Weight to handle data in the same cell.\n\n"
                      "cell value = previous + new * weight");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();
        ImGui::NewLine();

        purpleButton("Reset Parameters", resetParametersPressed, 130, 25);
        ImGui::NewLine();
    }
    if (ImGui::CollapsingHeader("Select Speed of Sound in Medium")) {
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

        addText("Velocity through medium: ");
        ImGui::SameLine();
        addText((std::to_string(velocity) + " m/s").c_str(), purple);
        addText("Frequency:               ");
        ImGui::SameLine();
        addText((std::to_string(freq) + " MHz").c_str(), purple);
        ImGui::NewLine();
    }

    if (ImGui::CollapsingHeader("Add Filters")) {
        ImGui::NewLine();
        ImGui::Indent();
        addText("Select Filter");
        ImGui::SameLine();
        //Select filter
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.00f));

        const char* items[] = { "Low Pass", "High Pass", "Band Pass", "Band Stop", "Moving Average" };
        static const char* current_item = NULL;

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.831f, .882f, .949f, 1.00f));
        if (ImGui::BeginCombo("##filters", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            ImGui::PopStyleColor(1);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.00f));
            for (auto & item : items)
            {
                bool is_selected = (current_item == item); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(item, is_selected))
                    current_item = item;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor(2);


        static float centerFreqLow = 4.5;
        static float centerFreqHigh = 4;
        static float centerFreqBP = 4.25;
        static float centerFreqStop = 4.5;
        static float bandWidthBP = 0.5;
        static float bandWidthStop = 0.5;
        static int windowSize = 10;

        ImGui::Indent();
        if(current_item!=NULL) {
            ImGui::NewLine();
            if(std::strcmp(current_item, "Low Pass") == 0) {
                addText("Low Pass Filter");
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::Indent();
                ImGui::SliderFloat("##lowCenter", &centerFreqLow, 0, 7.8);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();
                ImGui::Unindent();
            }
            else if(std::strcmp(current_item, "High Pass") == 0) {
                addText("High Pass Filter");
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::Indent();
                ImGui::SliderFloat("##highCenter", &centerFreqHigh, 0, 7.8);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();
                ImGui::Unindent();
            }
            else if(std::strcmp(current_item, "Band Pass") == 0) {
                addText("Band Pass Filter");
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);

                addText("Center Frequency: ");
                ImGui::Indent();
                ImGui::SliderFloat("##bpCenter", &centerFreqBP, 0, 7.8);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();

                addText("Band Width: ");
                ImGui::Indent();
                ImGui::SliderFloat("##bpWidth", &bandWidthBP, 0, centerFreqBP);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();

                ImGui::Unindent();
            }
            else if(std::strcmp(current_item, "Band Stop") == 0) {
                addText("Band Stop Filter");
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);

                addText("Center Frequency: ");
                ImGui::Indent();
                ImGui::SliderFloat("##stopCenter", &centerFreqStop, 0, 7.8);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();

                addText("Band Width: ");
                ImGui::Indent();
                ImGui::SliderFloat("##stopWidth", &bandWidthStop, 0, centerFreqStop);
                ImGui::SameLine(); addText(" MHz", purple);
                ImGui::Unindent();

                ImGui::Unindent();
            }
            else if(std::strcmp(current_item, "Moving Average") == 0) {
                addText("Moving Average Filter");
                ImGui::Indent();

                addText("Window Size: ");
                ImGui::Indent();
                ImGui::SliderInt("##window", &windowSize, 2, 100);
                ImGui::SameLine(); addText(" Cells", purple);
                ImGui::Unindent();

                ImGui::Unindent();
            }
        }

        bool addFilter = false;
        yellowButton("Add Filter", addFilter);

        ImGui::Unindent();

        ImGui::NewLine();

        if(current_item!=nullptr && addFilter) {
            if(std::strcmp(current_item, "Low Pass") == 0){
                filterList.push_back(1);
                filterList.push_back(centerFreqLow);
                filterList.push_back(-1);
            }
            else if(std::strcmp(current_item, "High Pass") == 0){
                filterList.push_back(2);
                filterList.push_back(centerFreqHigh);
                filterList.push_back(-1);
            }
            else if(std::strcmp(current_item, "Band Pass") == 0){
                filterList.push_back(3);
                filterList.push_back(centerFreqBP);
                filterList.push_back(bandWidthBP);
            }
            else if(std::strcmp(current_item, "Band Stop") == 0){
                filterList.push_back(4);
                filterList.push_back(centerFreqStop);
                filterList.push_back(bandWidthStop);
            }
            else if(std::strcmp(current_item, "Moving Average") == 0){
                filterList.push_back(5);
                filterList.push_back(windowSize);
                filterList.push_back(-1);
            }

            //reset to defaults
            centerFreqLow = 4.5;
            centerFreqHigh = 4;
            centerFreqBP = 4.25;
            centerFreqStop = 4.5;
            bandWidthBP = 0.5;
            bandWidthStop = 0.5;
            windowSize = 10;

            current_item = nullptr;
        }

        for (int x = 0; x < filterList.size(); x+=3) {
            int filterNumber = int(filterList.at(x));
            if(filterNumber == 1) {
                addText("Low Pass Filter", blue);
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+1)) + " MHz").c_str(), purple);
                ImGui::Unindent();
            }
            else if(filterNumber == 2) {
                addText("High Pass Filter", blue);
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+1)) + " MHz").c_str(), purple);
                ImGui::Unindent();
            }
            else if(filterNumber == 3) {
                addText("Band Pass Filter", blue);
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+1)) + " MHz").c_str(), purple);
                addText("Band Width: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+2)) + " MHz").c_str(), purple);
                ImGui::Unindent();
            }
            else if(filterNumber == 4) {
                addText("Band Stop Filter", blue);
                ImGui::Indent();
                addText("Butterworth filter");
                addText("Order: "); ImGui::SameLine(); addText("4", purple);
                addText("Center Frequency: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+1)) + " MHz").c_str(), purple);
                addText("Band Width: ");
                ImGui::SameLine();
                addText((std::to_string(filterList.at(x+2)) + " MHz").c_str(), purple);
                ImGui::Unindent();
            }
            else if(filterNumber == 5) {
                addText("Moving Average Filter", blue);
                ImGui::Indent();
                addText("Window Size: ");
                ImGui::SameLine();
                addText((std::to_string(int(filterList.at(x+1))) + " Cells").c_str(), purple);
                ImGui::Unindent();
            }
        }

        ImGui::NewLine();

        bool applied = false;
        bool clear = false;

        if(currState == 0) {
            yellowButton("Clear Filters", clear);
            if(clear) {
                filterList.clear();
            }
            purpleButton("Apply Filters", applied);
        } else if(currState == 1) {
            yellowButtonClicked("Clear Filters", clear);
            if(clear) {
                filterList.clear();
            }
            purpleButtonDisabled("Apply Filters", applied);
            addText("Applying Filters ... ", purple);
        } else if(currState == 2) {
            yellowButton("Clear Filters", clear);
            if(clear) {
                filterList.clear();
                currState = 0;
            }
            purpleButtonDisabled("Apply Filters", applied);
            addText("Successfully applied filters. Clear list to apply different filters.", purple);
        } else if(currState == 3) {
            yellowButton("Clear Filters", clear);
            if(clear) {
                filterList.clear();
                currState = 0;
            }
            purpleButton("Apply Filters", applied);
            addText("Error Applying Filters", orange);
        }

        if((currState == 0 || currState == 3) && applied && filterList.size() > 0) {
            //apply the filters
            applyFilter = true;
        }

        ImGui::Unindent();
    }

    static int id = 0;
    if (ImGui::CollapsingHeader("Marker Options")) {
        ImGui::NewLine();
        ImGui::Indent();
        addText("Select Marker Pair");
        ImGui::SameLine();
        //Select marker pair
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));

        static int current_marker_id = -1;
        std::vector<std::string> items;
        for (auto &marker : markerList) {
            items.push_back("Marker Pair " + std::to_string(marker.getNumber()));
        }
        items.emplace_back("+ Add Marker Pair");

//        glm::vec3 currColor = markerColors[current_marker_id % markerColors.size()];
        glm::vec3 tmpColor = current_marker_id == -1 ? glm::vec3(0, 0, 0) : markerList[current_marker_id].getColor();
        ImVec4 currColor = ImVec4(tmpColor.x, tmpColor.y, tmpColor.z, 1.00f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, currColor);
        if (ImGui::BeginCombo("##markerPair",
                              current_marker_id == -1 ? "" : (char *) ("Marker Pair " + std::to_string(
                                      markerList[current_marker_id].getNumber())).c_str())) // The second parameter is the label previewed before opening the combo.
        {
            fprintf(stdout, "MARKER HERE!!!\n");
            for (int n = 0; n < items.size(); n++) {
                glm::vec3 itemColor = (n == items.size() - 1 ? glm::vec3(0, 0, 0) : markerList[n].getColor());
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      n == items.size() - 1 ? ImVec4(1.0f, 1, 1, 1.00f) : ImVec4(itemColor.x,
                                                                                                 itemColor.y,
                                                                                                 itemColor.z, 1.00f));
                bool is_selected = (current_marker_id ==
                                    n); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable((char *) items[n].c_str(), is_selected)) {
                    if (n == items.size() - 1) {
                        //If the user adds a new marker
                        current_marker_id = items.size() - 1;
                        markerList.emplace_back(markerColors[current_marker_id % markerColors.size()], ++id);
                    } else {
                        current_marker_id = n;
                    }
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                ImGui::PopStyleColor();
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor(2);

        bool enableSnapTmp = false;
        if (enableSnap) {
            yellowButton("Disable Snap", enableSnapTmp);
            ImGui::SameLine();

            addText("          Snap Threshold");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::SliderInt("##snapThreshold", &snapThresholdIn, 0, 255);
            ImGui::PopItemWidth();

            if (enableSnapTmp)
                enableSnap = false;
        } else {
            yellowButton("Enable Snap", enableSnapTmp);
            if (enableSnapTmp)
                enableSnap = true;
        }
        ImGui::NewLine();


        //Load the marker options
        if (current_marker_id != -1 && current_marker_id != markerList.size()) {
            addText("Distance between markers: ");
            ImGui::SameLine();
            addText((std::to_string(markerList[current_marker_id].getDistance(freq, velocity, depth)) + " mm").c_str(), purple);

            //get marker positions
            Marker currMarker = markerList[current_marker_id];
            glm::vec3 marker1Pos = currMarker.getMarker1Pos();
            float marker1X = marker1Pos.x;
            float marker1Y = marker1Pos.y;
            float marker1Z = marker1Pos.z;

            glm::vec3 marker2Pos = currMarker.getMarker2Pos();
            float marker2X = marker2Pos.x;
            float marker2Y = marker2Pos.y;
            float marker2Z = marker2Pos.z;

            ImGui::NewLine();
            ImGui::Indent();
            addText("Click and drag markers to move, or use the sliders below", blue);
            ImGui::NewLine();

            ImGui::Indent();
            addText("Marker 1 Position");
            ImGui::Indent();
            ImGui::PushItemWidth(80);
            ImGui::SliderFloat("##marker1X", &marker1X, 0, 1);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("X     ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::SliderFloat("##marker1Y", &marker1Y, 0, 1);
            ImGui::SameLine();
            addText("Y     ");
            ImGui::SameLine();
            ImGui::SliderFloat("##marker1Z", &marker1Z, 0, 1);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("Z     ");
            ImGui::Unindent();
            ImGui::Unindent();
            ImGui::Unindent();

            ImGui::NewLine();
            ImGui::Indent();
            ImGui::Indent();
            addText("Marker 2 Position");
            ImGui::Indent();
            ImGui::PushItemWidth(80);
            ImGui::SliderFloat("##marker2X", &marker2X, 0, 1);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("X     ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::SliderFloat("##marker2Y", &marker2Y, 0, 1);
            ImGui::SameLine();
            addText("Y     ");
            ImGui::SameLine();
            ImGui::SliderFloat("##marker2Z", &marker2Z, 0, 1);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            addText("Z     ");
            ImGui::Unindent();
            ImGui::Unindent();

            //show/hide markers
            ImGui::NewLine();
            bool isShown = false;
            if (currMarker.getHidden()) {
                yellowButton("  Show  ", isShown);

                if (isShown)
                    markerList[current_marker_id].setHidden(false);
            } else {
                yellowButton("  Hide  ", isShown);

                if (isShown)
                    markerList[current_marker_id].setHidden(true);
            }

            ImGui::NewLine();
            bool remove = false;
            purpleButton("Remove Markers", remove, 120, 30);
            ImGui::NewLine();

            //update marker positions
            markerList[current_marker_id].setPositionMarker1(glm::vec3(marker1X, marker1Y, marker1Z));
            markerList[current_marker_id].setPositionMarker2(glm::vec3(marker2X, marker2Y, marker2Z));

            //remove marker
            if (remove) {
                markerList.erase(markerList.begin() + current_marker_id);
                current_marker_id = -1;
            }
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Scale Options")) {
        ImGui::NewLine();
        bool gridShown = false;
        if(scale.isGridShown()) {
            yellowButton("Hide Grid", gridShown);

            if(gridShown) {
                scale.showGrid(false);
            }
        } else {
            yellowButton("Show Grid", gridShown);

            if(gridShown) {
                scale.showGrid(true);
            }
        }

        addText("Scale shown in cm", blue);

        ImGui::Indent();
        addText("Scale X Location");
        ImGui::Indent();
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
        ImGui::Unindent();
        ImGui::Unindent();

        ImGui::Indent();
        addText("Scale X Location");
        ImGui::Indent();
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
        ImGui::Unindent();
        ImGui::Unindent();

        ImGui::Indent();
        addText("Scale X Location");
        ImGui::Indent();
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
        ImGui::Unindent();
        ImGui::Unindent();

    }
    if (ImGui::CollapsingHeader("Annotate Screen")) {
        bool isAnnotating = false;
        yellowButton("  Clear All Text  ", isAnnotating);
        if (isAnnotating) {
            textList.clear();
        }

        {
            addText("Enter text to add to screen: ");
            static bool is3D = true;
            bool clicked;
            if (is3D) {
                yellowButton("3D Text", clicked);

                if (clicked) is3D = false;
            } else {
                yellowButtonClicked("3D Text", clicked);

                if (clicked) is3D = true;
            }

            static std::string textInput;
            textInput.reserve(100);
            static int buf_size = textInput.capacity();
//            std::cout<<buf_size<<std::endl;

            if(strlen(textInput.c_str()) >= textInput.capacity()-1){
                std::cout<<"MAX SIZE REACHED!!"<<std::endl;
                textInput.reserve(textInput.capacity()*2);
                buf_size = textInput.capacity();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
            ImGui::InputTextMultiline("##custom", (char *) textInput.c_str(), buf_size);
            ImGui::PopStyleColor();

            bool isAddText = false;
            purpleButton("Add Text", isAddText);

            if(isAddText) {
                textList.emplace_back(textInput.c_str(), scr_width, scr_height, is3D);
            }

            bool isRemoveText = false;
            yellowButton("Remove Last Added Text", isRemoveText);
            if(isRemoveText) {
                textList.pop_back();
            }

            int id = 0;
            for(auto& txt: textList) {

                glm::vec3 pos = txt.getPos();
                float x = pos.x;
                float y = pos.y;
                float z = pos.z;

                if(!txt.getIs3D()){
                    x = txt.getRasterX();
                    y = txt.getRasterY();
                }

                addText(txt.getText().c_str());
                ImGui::Indent();
                addText("Text Position", purple);

                ImGui::Indent();

                if(txt.getIs3D()) {
                    ImGui::PushItemWidth(80);
                    ImGui::SliderFloat((std::string("##textX")+std::to_string(id)).c_str(), &x, -10, 10);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    addText("X     ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(80);
                    ImGui::SliderFloat((std::string("##textY")+std::to_string(id)).c_str(), &y, -10, 10);
                    ImGui::SameLine();
                    addText("Y     ");
                    ImGui::SameLine();
                    ImGui::SliderFloat((std::string("##textZ") + std::to_string(id)).c_str(), &z, -10, 10);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    addText("Z     ");
                }
                ImGui::Unindent();
                ImGui::Unindent();

                //update marker positions
                if(txt.getIs3D())
                    txt.setPos(glm::vec3(x, y, z));
                id++;
            }
        }
    }
    if (ImGui::CollapsingHeader("Fit Object")) {
        drawObjectMode = true;

        bool selectingSphere = false;
        bool selectingCylinder = false;
//        float size = myObj.getSize();
        if(!isDrawingBox) {
            yellowButton("Fit Sphere", selectingSphere);
            yellowButton("Fit Cylinder", selectingCylinder);


            addText("Box Size: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##selectBoxSize", myObj.getSize(), 0.1, 5);
            ImGui::PopItemWidth();

            if(selectingSphere) {
                isDrawingBox = true;
                myObj.calculateSphere();
            } else if (selectingCylinder) {
                isDrawingBox = true;
                myObj.calculateCylinder();
            }
        }
        else {
            yellowButtonClicked("Select Area", selectingSphere);

            if(myObj.getDisplayObject() == 0) {
                //sphere
                ImGui::NewLine();
                addText("Center: ");
                ImGui::Indent();
                glm::vec3 objPos = myObj.getPosWorld(freq, velocity, depth);
                addText(std::to_string(objPos.x).c_str());
                ImGui::SameLine();
                addText(std::to_string(objPos.y).c_str());
                ImGui::SameLine();
                addText(std::to_string(objPos.z).c_str());
                ImGui::Unindent();
                addText("Radius: ");
                ImGui::Indent();
                addText((std::to_string(myObj.getRadius(freq, velocity, depth)) + " mm").c_str());
                ImGui::Unindent();
                addText("Threshold Value: ");
                ImGui::Indent();
                addText(std::to_string(myObj.getThreshold()).c_str());
                ImGui::Unindent();
            } else if(myObj.getDisplayObject() == 1) {
                //cylinder
                ImGui::NewLine();
                addText("Center: ");
                ImGui::Indent();
                glm::vec3 objPos = myObj.getPosWorld(freq, velocity, depth);
                addText(std::to_string(objPos.x).c_str());
                ImGui::SameLine();
                addText(std::to_string(objPos.y).c_str());
                ImGui::SameLine();
                addText(std::to_string(objPos.z).c_str());
                ImGui::Unindent();
                addText("Radius: ");
                ImGui::Indent();
                addText((std::to_string(myObj.getRadius(freq, velocity, depth)) + " mm").c_str());
                ImGui::Unindent();
                addText("Direction: ");
                ImGui::Indent();
                glm::vec3 dir = myObj.getDirection();
                addText(("<"+ std::to_string(dir.x) + ", " + std::to_string(dir.y) + ", "+ std::to_string(dir.z)+">").c_str());
                ImGui::Unindent();
                addText("Threshold Value: ");
                ImGui::Indent();
                addText(std::to_string(myObj.getThreshold()).c_str());
                ImGui::Unindent();
            }

            if(selectingSphere) {
                isDrawingBox = false;
                myObj.selectArea();
            }
        }

    } else {
        drawObjectMode = false;
    }
    ImGui::PopStyleColor();
    ImGui::End();
}


/**
 * Returns a list of all the files located in the path.
 * @param pathIn folder to check for files
 * @param filterState filterState = 0: no filter, 1 = submarine files, 2 = whitefin files
 * @return List of all files located in the path
 */
std::vector<std::string> getFileDirectories(boost::filesystem::path pathIn, int filterState = 0) {
    using namespace boost::filesystem;

    std::vector<std::string> items;
    for (directory_iterator itr(pathIn); itr != directory_iterator(); ++itr) {
        items.push_back((itr->path().filename()).string());
    }
    return items;
}

/**
 * ImGUI panel to load a data file.
 * @param filePath folder to load data file from
 * @param file pointer for the file to load
 * @param depth depth of data to load
 * @param gain gain for time gain control
 * @param weight weight for update weight in DensityMap
 * @param load Set to true if user clicks load. False otherwise.
 * @param currState int to describe what state the load file is in. 0 = no message, 1 = loading, 2 = success, 3 = error
 * @param errorMessage Error Message to display if currState = 3.
 */
void loadDataFromFile(
        boost::filesystem::path filePath,
        std::string &file,
        int &depth, float &gain, float &weight,
        bool &load,
        int currState, std::string errorMessage
) { // loadDataFromFile(file, depth, gain, weight, error)
//    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT));
    ImGui::Begin("Load Data from File");

    addText("Load Data from File", ImColor(0, 0, 0, 255));
    ImGui::Indent();
    addText("Select File");
    ImGui::Indent();

    addText("Filter by Probe Type: ");
    ImGui::SameLine();
    bool submarineFilter = false;
    bool whiteFinFilter = false;
    static int filterType = 0;

    if (filterType == 0) {
        yellowButton("Submarine", submarineFilter);
        ImGui::SameLine();
        yellowButton("White Fin", whiteFinFilter);

        if (submarineFilter) filterType = 1;
        if (whiteFinFilter) filterType = 2;
    } else if (filterType == 1) {
        yellowButtonClicked("Submarine", submarineFilter);
        ImGui::SameLine();
        yellowButton("White Fin", whiteFinFilter);

        if (submarineFilter) filterType = 0;
        if (whiteFinFilter) filterType = 2;
    } else if (filterType == 2) {
        yellowButton("Submarine", submarineFilter);
        ImGui::SameLine();
        yellowButtonClicked("White Fin", whiteFinFilter);

        if (submarineFilter) filterType = 1;
        if (whiteFinFilter) filterType = 0;
    }

    //Select File Directory
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));

    std::vector<std::string> items = getFileDirectories(filePath / boost::filesystem::path("data"), filterType);
    static std::string current_item;
    if (currState == 4) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
    if (ImGui::BeginCombo("##fileSelector",
                          current_item.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1, 1, 1.00f));
        for (auto &item : items) {
            const char *currItem = (char *) item.c_str();
            bool is_selected = (current_item ==
                                currItem); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(currItem, is_selected))
                current_item = currItem;
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    if (currState == 4) ImGui::PopStyleColor();
    if (!current_item.empty())
        file.assign(current_item);

    ImGui::Unindent();
    ImGui::Unindent();
    ImGui::NewLine();

    addText("Set Display Parameters");
    ImGui::Indent();
    addText("Depth");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderInt("##depth", &depth, 1, 2500);
    createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
    ImGui::PopItemWidth();
    ImGui::Unindent();
    ImGui::Unindent();
    ImGui::NewLine();

    ImGui::Indent();
    addText("Gain");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("##gain", &gain, 0, 5);
    createToolTip("Time gain compensation value.\n"
                  "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
    ImGui::PopItemWidth();
    ImGui::Unindent();
    ImGui::Unindent();
    ImGui::NewLine();

    ImGui::Indent();
    addText("Weight");
    ImGui::Indent();
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("##weight", &weight, 0, 1);
    createToolTip("Weight to handle data in the same cell.\n\n"
                  "cell value = previous + new * weight");
    ImGui::PopItemWidth();
    ImGui::Unindent();
    ImGui::Unindent();
    ImGui::NewLine();

    ImGui::PopStyleColor();

    ImGui::NewLine();
    ImGui::NewLine();

    if (currState == 0) {
        purpleButton("Load", load);
    } else if (currState == 1) {
        //loading
        purpleButtonDisabled("Load", load);
        ImGui::SameLine();
        addText("Loading...", purple);
        //disable loadButton...
    } else if (currState == 2) {
        //success!
        purpleButton("Load", load);
        ImGui::SameLine();
        addText((file + " sucessfully loaded").c_str(), blue);
    } else if (currState == 3) {
        purpleButton("Load", load);
        ImGui::SameLine();
//        addText("ERROR", orange);
//        addText("=======================", orange);
        addText(errorMessage.c_str(), orange);
    } else if (currState == 4) {
        purpleButton("Load", load);
        ImGui::SameLine();
        addText("Select a file to load", orange);
    }

    ImGui::End();
}

/**
 * ImGUI panel to scan from a probe.
 * @param probeIP IP address of the probe
 * @param probeUsername username of the probe
 * @param probePassword password of the probe
 * @param compIP IP address of the computer
 * @param depth depth parameter
 * @param gain gain parameter
 * @param weight weight parameter
 * @param isSubmarine Set to true if connecting to Submarine probe. Set to false if connecting to White Fin.
 * @param isDefault Set to true if in default connection. False for advanced connection.
 * @param lxRangeMin Lx-16 minimum range value input
 * @param lxRangeMax Lx-16 maximum range value input
 * @param lxRes Lx-16 resolution input
 * @param servoRangeMin Servo minimum range input
 * @param servoRangeMax Servo maximum range input
 * @param servoRes Servo resolution input
 * @param customCommand Custom command to send to Red Pitaya if in advanced connection mode.
 * @param liveScan Set to true if user clicks the Live Scan button.
 * @param scanToFile Set to true if user clicks save to file button.
 * @param sendCustom Set to true if user clicks the Send Custom button.
 * @param currState int to determine the current state (nothing, loading, error, success)
 *  0 = first, 1 = loading, 2 = success,
    3 = error connection,
    4 = probeIP error, 5 = probeUsername error, 6 = probePassword error
    7 = compIP error
    8 = lxMin error, 9 = lxMax error, 10 = lxMin > lxMax, 11 = lxRes error
    12 = servoMin error, 13 = servoMax error, 14 = servoMin > servoMax, 15 = servoRes error
 * @param output The output received from the probe to display.
 * @param errorMessage error message to display if error occurred.
 * @param errorSaveFile 0 = user didn't click save file, 1 = success! -1 = failed to save file
 */
void scanFromProbe(
        std::string *probeIP, std::string *probeUsername, std::string *probePassword, std::string *compIP,
        int& depth, float& gain, float& weight,
        bool &isSubmarine, bool &isDefault,
        float &lxRangeMin, float &lxRangeMax, int &lxRes,
        float &servoRangeMin, float &servoRangeMax, int &servoRes,
        std::string *customCommand,
        bool &liveScan, bool &scanToFile, bool &sendCustom,
        int &currState,
        std::string &output, std::string &errorMessage, int errorSaveFile
) {
    ImGui::Begin("Scan From Probe");

    static std::string lxRangeMinInput = std::to_string(lxRangeMin);
    static std::string lxRangeMaxInput = std::to_string(lxRangeMax);
    static std::string lxResInput = std::to_string(lxRes);
    static std::string servoRangeMinInput = std::to_string(servoRangeMin);
    static std::string servoRangeMaxInput = std::to_string(servoRangeMax);
    static std::string servoResInput = std::to_string(servoRes);

    if (currState == 2)
        ImGui::SetNextTreeNodeOpen(false);
    else
        ImGui::SetNextTreeNodeOpen(true);

    //connection output
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.00f));

    if (ImGui::CollapsingHeader("Connect to Probe")) {
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));

        addText("Enter addresses for the probe and computer to establish a connection.", blue);
        ImGui::NewLine();
        addText("Probe");
        ImGui::Indent();

        addText("IP Address: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (currState == 4) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
        ImGui::InputText("##ipAddressProbe", (char *) probeIP->c_str(), probeIP->capacity() + 1,
                         (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
        if (currState == 4) ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        createToolTip("Probe IP Address Finding Instructions");

        addText("username:   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (currState == 5) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
        ImGui::InputText("##username", (char *) probeUsername->c_str(), probeUsername->capacity() + 1,
                         (currState == 1) ? INPUT_TEXT_READ_ONLY : 0);
        if (currState == 5) ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        createToolTip("Default is set to 'root'");

        addText("password:   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (currState == 6) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
        ImGui::InputText("##password", (char *) probePassword->c_str(), probePassword->capacity() + 1,
                         (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_PASSWORD);
        if (currState == 6) ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        createToolTip("Default is set to 'root'");

        ImGui::Unindent();
        ImGui::NewLine();
        addText("Computer");
        ImGui::Indent();

        addText("IP Address: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (currState == 7) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
        ImGui::InputText("##ipAddressComp", (char *) compIP->c_str(), compIP->capacity() + 1,
                         (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
        if (currState == 7) ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        createToolTip(
                "Run the following commands on your terminal to find the IP address based on your operating system.\n\n"
                "Windows:   ipconfig | findstr IPv4\n"
                "Mac/Linux: ifconfig | grep inet");
        ImGui::Unindent();

        ImGui::NewLine();
        ImGui::NewLine();
        addText("Select Probe Type: ");
        ImGui::SameLine();

        bool submarine = false;
        bool whiteFin = false;
        if (currState != 1 && isSubmarine) {
            yellowButtonClicked("Submarine", submarine);
            ImGui::SameLine();
            yellowButton("White Fin", whiteFin);

            if (whiteFin)
                isSubmarine = false;
        } else if (currState != 1) {
            yellowButton("Submarine", submarine);
            ImGui::SameLine();
            yellowButtonClicked("White Fin", whiteFin);

            if (submarine)
                isSubmarine = true;
        }

        ImGui::NewLine();

        addText("Set Display Parameters");
        ImGui::Indent();
        addText("Depth");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("##depth", &depth, 1, 2500);
        createToolTip("Depth of each scan line to display. (By default the probe collects 2500 values per scan)");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();

        ImGui::Indent();
        addText("Gain");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##gain", &gain, 0, 5);
        createToolTip("Time gain compensation value.\n"
                      "To overcome ultrasound attenuation by increasing signal gain as time passes from emitted wave.");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();

        ImGui::Indent();
        addText("Weight");
        ImGui::Indent();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##weight", &weight, 0, 1);
        createToolTip("Weight to handle data in the same cell.\n\n"
                      "cell value = previous + new * weight");
        ImGui::PopItemWidth();
        ImGui::Unindent();
        ImGui::Unindent();
        ImGui::NewLine();

        ImGui::NewLine();

        bool defaultCon = false;
        bool advancedCon = true;
        if (currState != 1 && isDefault) {
            yellowButtonClicked("   Default Connection   ", defaultCon);
            ImGui::SameLine();
            yellowButton("   Advanced Connection   ", advancedCon);

            if (advancedCon)
                isDefault = false;
        } else if (currState != 1) {
            yellowButton("   Default Connection   ", defaultCon);
            ImGui::SameLine();
            yellowButtonClicked("   Advanced Connection   ", advancedCon);

            if (defaultCon)
                isDefault = true;
        }
        ImGui::NewLine();
        ImGui::PopStyleColor();

        if (isDefault) {
            //White Fin Option
            if (!isSubmarine) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
                ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 10);
                addText("Use default connection to make scans with the probe with the following parameter settings.",
                        blue);
                ImGui::PopTextWrapPos();
                ImGui::NewLine();
                addText("Probe Scan Settings");

                ImGui::Indent();
                addText("Lx-16");

                ImGui::Indent();
                addText("Range:      ");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                if (currState == 8 || currState == 10) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##lxMin", (char *) lxRangeMinInput.c_str(), lxRangeMinInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 8 || currState == 10) ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                addText("to");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                if (currState == 9 || currState == 10) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##lxMax", (char *) lxRangeMaxInput.c_str(), lxRangeMaxInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 9 || currState == 10) ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                addText("degrees");

                addText("Resolution: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                if (currState == 11) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##lxRes", (char *) lxResInput.c_str(), lxResInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 11) ImGui::PopStyleColor();
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
                if (currState == 12 || currState == 14) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##servoMin", (char *) servoRangeMinInput.c_str(), servoRangeMinInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 12 || currState == 14) ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                addText("to");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                if (currState == 13 || currState == 14) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##servoMax", (char *) servoRangeMaxInput.c_str(), servoRangeMaxInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 13 || currState == 14) ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                addText("degrees");

                addText("Resolution: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                if (currState == 15) ImGui::PushStyleColor(ImGuiCol_FrameBg, orange);
                ImGui::InputText("##servoRes", (char *) servoResInput.c_str(), servoResInput.capacity() + 1,
                                 (currState == 1) ? INPUT_TEXT_READ_ONLY : INPUT_TEXT_CHARS_DECIMAL);
                if (currState == 15) ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                addText("steps");

                ImGui::Unindent();
                ImGui::Unindent();
                ImGui::PopStyleColor();
            }

            ImGui::NewLine();
            ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 10);
            addText("Use the Live Scan option to  enter live mode and visualize scans. Use Scan to File option to scan and save into a .dat file.",
                    purple);
            ImGui::PopTextWrapPos();

            ImGui::NewLine();
            purpleButton("Live Scan", liveScan);
//        ImGui::SameLine();
//        purpleButton("Scan to File", scanToFile);

        } else {
            addText("Enter the command to send to the probe below", blue);
            addText("*Only use if you're certain you know what you're doing.", orange);
            ImGui::NewLine();
            addText("Custom Command: ");

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0, 0, 1.00f));
            ImGui::InputTextMultiline("##custom", (char *) customCommand->c_str(), customCommand->capacity() + 1);
            ImGui::PopStyleColor();

            ImGui::NewLine();
            purpleButton("Live Scan", sendCustom);
        }
    } else {
        ImGui::PopStyleColor();
    }

    //add user messages
    ImGui::NewLine();
    if (currState == 1)
        addText("Connecting to probe...", purple);
    if (currState == 2)
        addText("Successfully connected to probe", blue);
    if (currState == 3) {
        addText("ERROR connecting to probe", orange);
        addText("=========================", orange);
        addText(std::string("     " + errorMessage).c_str(), orange);
    }
    if (currState == 4)
        addText("Probe IP cannot be empty", orange);
    if (currState == 5)
        addText("Probe username cannot be empty", orange);
    if (currState == 6)
        addText("Probe password cannot be empty", orange);
    if (currState == 7)
        addText("Computer IP cannot be empty", orange);
    if (currState == 8)
        addText("Lx-16 min range should be a number greater than or equal to -180", orange);
    if (currState == 9)
        addText("Lx-16 max range should be a number less than or equal to 180", orange);
    if (currState == 10)
        addText("Lx-16 max range should be greater than the min value", orange);
    if (currState == 11)
        addText("Lx-16 resolution should be between 1 - 200", orange);
    if (currState == 12)
        addText("Servo min range should be a number greater than or equal to -30", orange);
    if (currState == 13)
        addText("Servo max range should be a number less than or equal to 30", orange);
    if (currState == 14)
        addText("Servo max range should be greater than the min value", orange);
    if (currState == 15)
        addText("Servo resolution should be between 1 - 200", orange);

//    ImGui::NewLine();

    //connection output
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.00f));

    if (currState == 2)
        ImGui::SetNextTreeNodeOpen(true);

    if (ImGui::CollapsingHeader("Connection Output")) {
        ImGui::PopStyleColor();
        ImGui::NewLine();
        purpleButton("Save Scan", scanToFile);
        ImGui::NewLine();
        if(errorSaveFile == -1){
            addText("========= ERROR SAVING FILE ========");
            addText(std::string("     " + errorMessage).c_str(), orange);
        } else if(errorSaveFile == 1) {
            addText("Successfully saved file");
        }
        addText("Probe output: ");
        ImGui::Indent();
        addText(output.c_str());
    } else {
        ImGui::PopStyleColor();
    }

//  if user clicks a send button, then get the input values
    if (!isSubmarine && (liveScan || scanToFile)) {

        //update lx-16 values. If conversion fails, set value to one out of range
        try {
            lxRangeMin = std::stof(lxRangeMinInput);
        } catch (...) { lxRangeMin = -1000; }

        try {
            lxRangeMax = std::stof(lxRangeMaxInput);
        } catch (...) { lxRangeMax = 1000; }
        try {
            lxRes = std::stoi(lxResInput);
        } catch (...) { currState = 0; }

        //update servo values
        try {
            servoRangeMin = std::stof(servoRangeMinInput);
        } catch (...) { currState = -1000; }
        try {
            servoRangeMax = std::stof(servoRangeMaxInput);
        } catch (...) { currState = 1000; }
        try {
            servoRes = std::stoi(servoResInput);
        } catch (...) { currState = 0; }
    }

    ImGui::End();
}

void GUI::drawProbe(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY) {
    if (isProbeLoaded) {
        //draw the probe
        probe.draw(projection, view, rotationX, rotationY);
    }

    if (isDataLoaded && !isProbeLoaded) {
        if (probeType == 0)
            probe.loadNewProbe("config_file/models/PROBE_CENTERED.stl");
        else
            probe.loadNewProbe("config_file/models/WHITE_FIN_CENTERED.stl");
        isProbeLoaded = true;
    }
    if (!isDataLoaded) {
        isProbeLoaded = false;
    }
}

//Draws the ImGui widgets on the screen
void GUI::drawWidgets(glm::mat4 projection, glm::mat4 view) {
    // render your IMGUI
    if (renderedScreen == 0)
        drawOpenFrame(screen0Load, screen0Scan, width, height);
    else if (renderedScreen == 1)
        loadDataFromFile(filePath, screen1File, dispDepth, dispGain, dispWeight, screen1Load, screen1CurrState,
                         screen1ErrorMessage);
    else if (renderedScreen == 2)
        scanFromProbe(&screen2ProbeIP, &screen2ProbeUsername, &screen2ProbePassword, &screen2CompIP,
                dispDepth, dispGain, dispWeight,
                screen2IsSub,
                      screen2IsDefault,
                      screen2LxMin, screen2LxMax, screen2LxRes, screen2ServoMin, screen2ServoMax, screen2ServoRes,
                      &screen2CustomCommand,
                      screen2LiveScan, screen2ScanToFile, screen2SendCustom,
                      screen2CurrState, screen2Output, screen2ErrorMessage, screen2SaveFileState
        );
    if (isDataLoaded) {
        displaySettings(
                dispDepth, dispGain, dispWeight, dispBrightness, dispContrast, dispCutoff, dispZoom,
                dispReset,
                mediumActive, dispVel, dispFreq, inputVel,
                scaleXY, scaleXZ, scaleYX, scaleYZ, scaleZX, scaleZY, scale, markers, myTexts,
                filterList, applyFilters, applyFilterState,
                snap, snapThreshold, myObj,
                width, height
        );
    }
}

void GUI::interactionHandler() {
    if (renderedScreen == 0) {
        if (screen0Load == 1) {
            renderedScreen = 1;
            isLoadFile = true;
        } else if (screen0Scan == 1) {
            renderedScreen = 2;
            isLoadFile = false;
        }
    }
    if (renderedScreen == 1) {
        if (screen1Error) {
            screen1CurrState = 3;
            return;
        }
        if (screen1Load) {
            //first time clicking load
            if (screen1CurrState != 1) {
                //check to make sure file is selected
                if (screen1File.empty()) {
                    screen1CurrState = 4;
                    return;
                }

                //reload file
                gridPointer->clear();
                screen1DataUpdate = false;
                isDataLoaded = false;
                screen1Error = false;

                screen1CurrState = 1;

                std::string fileName = (filePath / boost::filesystem::path("data/" + screen1File)).string();
                //loadFile pointer function from main --> will only have 1 load file now with new data type
                bool noError = readDataMain(*gridPointer, fileName,
                                            dispGain, dispDepth, screen1DataUpdate, screen1ErrorMessage, probeType,
                                            screen1Error);
                gridPointer->setUpdateCoefficient(dispWeight);

                if (!noError) {
                    screen1CurrState = 3;
                }
            }
        } else {
            if (screen1DataUpdate) {
                screen1CurrState = 2;
                isDataLoaded = true;
            }
        }
    }
    if (renderedScreen == 2) {
        if (screen2ErrorSetUp) {
            screen2CurrState = 3;
            return;
        }
        if (screen2ErrorSaveFile) {
            screen2SaveFileState = -1;
        }
        try {
            if (screen2LiveScan && screen2CurrState != 1) {
                fprintf(stdout, "==========================================================\n");
                //error handling
                if (!passErrorCheckingScreen2()) return;

                screen2Connected = false;
                screen2CurrState = 1;
                gridPointer->clear();
                isDataLoaded = false;
                screen2ErrorSetUp = false;
                screen2LiveScan = false;

                //set the gain and depth in data before connecting to probe
                setGainMain(dispGain);
                setDepthMain(dispDepth);
                gridPointer->setUpdateCoefficient(dispWeight);

                bool noError = connectToProbeMain(*gridPointer, screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                  screen2ProbePassword.c_str(),
                                                  screen2CompIP.c_str(),
                                                  screen2IsSub, screen2LxMin, screen2LxMax,
                                                  screen2LxRes,
                                                  screen2ServoMin, screen2ServoMax,
                                                  screen2ServoRes,
                                                  "", 0, screen2Output, screen2Connected, screen2ErrorSetUp,
                                                  screen2ErrorMessage);
                if (!noError) screen2CurrState = 3;

                probeType = screen2IsSub ? 0 : 1;
            }
            if (screen2SendCustom && screen2CurrState != 1) {
                //error handling
                if (!passErrorCheckingScreen2()) return;

                screen2Connected = false;
                screen2CurrState = 1;
                gridPointer->clear();
                isDataLoaded = false;
                screen2ErrorSetUp = false;
                screen2SendCustom = false;

                //set the gain and depth in data before connecting to probe
                setGainMain(dispGain);
                setDepthMain(dispDepth);
                gridPointer->setUpdateCoefficient(dispWeight);

                bool noError = connectToProbeMain(*gridPointer, screen2ProbeIP.c_str(), screen2ProbeUsername.c_str(),
                                                  screen2ProbePassword.c_str(), screen2CompIP.c_str(),
                                                  screen2IsSub, 0, 0, 0,
                                                  0, 0, 0,
                                                  screen2CustomCommand.c_str(), 2, screen2Output, screen2Connected,
                                                  screen2ErrorSetUp, screen2ErrorMessage);
                if (!noError) screen2CurrState = 3;

                probeType = screen2IsSub ? 0 : 1;
            }
        }
        catch (...) {
            //connection error
            screen2CurrState = 3;
        }

        if (screen2Connected) {
            screen2CurrState = 2;
            isDataLoaded = true;
        }

        //User wants to save the collected files
        if (screen2Connected && screen2ScanToFile) {
            screen2SaveFileState = 1;
            //call save file in data...
            saveFileMain(screen2IsSub, screen2ErrorSaveFile, screen2ErrorMessage, false);
        }
    }

    if(applyFilters) {
        applyFilters = false;
        //reload file
        gridPointer->clear();
        applyFiltersUpdated = false;

        applyFilterState = 1; //loading

        std::string fileName = (filePath / boost::filesystem::path("data/" + screen1File)).string();

        //loadFile pointer function from main --> will only have 1 load file now with new data type
        bool noError = applyFiltersMain(*gridPointer, fileName,
                                    dispGain, dispDepth, applyFiltersUpdated, filterList);
        gridPointer->setUpdateCoefficient(dispWeight);

    } else if(applyFiltersUpdated) {
        applyFilterState = 2;
        applyFiltersUpdated = false;
    }

    if (isDataLoaded) {
        //display parameters
        if (dispReset) {
            reset();
            dispReset = false;
        }

        gridPointer->setBrightness(dispBrightness);
        gridPointer->setThreshold(dispCutoff);
        gridPointer->setContrast(dispContrast);

        setZoomMain(dispZoom);

        //set speed of sound
        if (mediumActive == 0) dispVel = 1102;
        if (mediumActive == 1) dispVel = 1538;
        if (mediumActive == 2) dispVel = atof(inputVel.c_str());
    }
}

bool GUI::passErrorCheckingScreen2() {
    if (strcmp(screen2ProbeIP.c_str(), "") == 0) {
        screen2CurrState = 4;
        return false;
    }
    if (strcmp(screen2ProbeUsername.c_str(), "") == 0) {
        screen2CurrState = 5;
        return false;
    }
    if (strcmp(screen2ProbePassword.c_str(), "") == 0) {
        screen2CurrState = 6;
        return false;
    }
    if (strcmp(screen2CompIP.c_str(), "") == 0) {
        screen2CurrState = 7;
        return false;
    }

    //if the user is running white fin in default mode, then check values
    if (screen2IsDefault && !screen2IsSub) {
        //lx values
        if (screen2LxMin < -180) {
            screen2CurrState = 8;
            return false;
        }
        if (screen2LxMax > 180) {
            screen2CurrState = 9;
            return false;
        }
        if (screen2LxMin > screen2LxMax) {
            screen2CurrState = 10;
            return false;
        }
        if (screen2LxRes > 200 || screen2LxRes < 1) {
            screen2CurrState = 11;
            return false;
        }
        //servo values
        if (screen2ServoMin < -30) {
            screen2CurrState = 12;
            return false;
        }
        if (screen2ServoMax > 30) {
            screen2CurrState = 13;
            return false;
        }
        if (screen2ServoMin > screen2ServoMax) {
            screen2CurrState = 14;
            return false;
        }
        if (screen2ServoRes > 200 || screen2ServoRes < 1) {
            screen2CurrState = 15;
            return false;
        }
    }

    return true;
}

bool GUI::mouseOnObjects(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen) {
    intersectedMarker = nullptr;

    //check for marker intersections
    float minT = -1;
    for (auto &marker: markers) {
        float tmpT = -1;
        int tmpMarker = -1;
        marker.setIntersected(-1);

        tmpMarker = marker.checkMouseOnMarker(rayOrigin, rayDirection, tmpT);
        if (tmpMarker != -1) {
            if (minT == -1 || (tmpT < minT && tmpT != -1)) {
                intersectedMarker = &marker;
                intersectedMarkerNum = tmpMarker;
                minT = tmpT;
            }
        }
    }

    //check for text intersections
    intersectedText = nullptr;
    for (auto &text: myTexts) {
        float tmpT = -1;

        text.setIntersected(false);

        if (text.checkMouseOnText(rayOrigin, rayDirection, tmpT, xPosScreen, yPosScreen) != -1) {
            if (minT == -1 || (tmpT < minT && tmpT != -1)) {
                intersectedText = &text;
                minT = tmpT;
            }
        }
    }

    float tmpT = -1;
    myObj.setIntersected(false);
    if(drawObjectMode && myObj.checkMouseOnCube(rayOrigin, rayDirection, tmpT) != -1) {
        if(minT == -1 || (tmpT < minT && tmpT != -1)) {
            intersectedText = nullptr;
            intersectedMarker = nullptr;
            myObj.setIntersected(true);
            minT = tmpT;
        }
    }

    if (intersectedText != nullptr) {
        intersectedMarker = nullptr;
    }

    if (minT == -1) {
        showMarkerDistance = false;
        return false;
    }

    if (intersectedMarker != nullptr) {
        intersectedMarker->setIntersected(intersectedMarkerNum);

        showMarkerDistance = true;

        markerXPos = xPosScreen;
        markerYPos = yPosScreen;
    } else if (intersectedText != nullptr) {
        intersectedText->setIntersected(true);
    }

    return true;
}

/**
 * Finds the intersection of a ray with a cube and sets the starting t and ending t for the two intersections.
 * @param rayOriginGrid ray origin (e)
 * @param rayDirectionGrid ray direction (d)
 * @param tmin Satisfies the first point of intersection: e + t_min * d = P1
 * @param tmax Satisfies the second point of intersection: e + t_max * d = P2
 * @return true for intersection, false otherwise
 */
bool GUI::intersectGrid(glm::vec3 rayOriginGrid, glm::vec3 rayDirectionGrid, float &tmin, float &tmax) {
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
glm::vec3 GUI::getSnapPoint(glm::vec3 rayOrigin, glm::vec3 currPoint) {

    float tmin, tmax;

    glm::vec3 rayDirection = glm::normalize(currPoint - rayOrigin);

    //Transform to grid coordinates
    glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0, 0, 0, 1));
    glm::vec3 rayOriginGrid = glm::transpose(rotation) * glm::vec4(rayOrigin.x, rayOrigin.y, rayOrigin.z, 1);
    glm::vec3 rayDirectionGrid =
            glm::transpose(rotation) * glm::vec4(rayDirection.x, rayDirection.y, rayDirection.z, 1);

    //Get intersection t values for ray with DensityMap grid in grid coordinates
    if (!intersectGrid(rayOriginGrid, rayDirectionGrid, tmin, tmax))
        return currPoint;

    //convert grid coordinates to grid cell values [0 - 1]
    glm::vec3 p0 = (rayOriginGrid + tmin * rayDirectionGrid + glm::vec3(5, 5, 5)) / 10.0;
    glm::vec3 p1 = (rayOriginGrid + tmax * rayDirectionGrid + glm::vec3(5, 5, 5)) / 10.0;

    //get point to snap to in grid cell values
    glm::vec3 destP = getSnapPointGrid(p0, p1, 100);

    //If nothing to snap to, then get original point
    if (destP.x == -1 && destP.y == -1 && destP.z == -1)
        return currPoint;

    //convert grid cell coordinates to grid coordinates
    destP = destP * 10.0 - glm::vec3(5, 5, 5);
    //convert grid coordinates to world coordinates
    destP = modelWorld * glm::vec4(destP.x, destP.y, destP.z, 1);

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
        auto val = (float) gridPointer->readCellInterpolated(x, y, z);

        if (val > snapThreshold)
            return glm::vec3(x, y, z);

        // Move x, y, and z along the line
        x += dx;
        y += dy;
        z += dz;
    }

    //if none meets criteria, return null point
    return glm::vec3(-1, -1, -1);
}


void GUI::moveObject(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen) {
    if(intersectedMarker != nullptr) {
        moveMarker(rayOrigin, rayDirection, xPosScreen, yPosScreen);
    }
    else if(intersectedText != nullptr) {
        moveText(rayOrigin, rayDirection, xPosScreen, yPosScreen);
    }
    else if(myObj.getIntersected()) {
        moveMeasureObject(rayOrigin, rayDirection, xPosScreen, yPosScreen);
    }
}

void GUI::moveText(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen) {
    if(intersectedText->getIs3D()){
        glm::vec3 textPos = intersectedText->getPos();

        glm::vec3 v0 = modelWorld * (glm::vec4(textPos.x, textPos.y, textPos.z, 1));
        glm::vec4 normal = glm::vec4(0, 0, 1, 0);
        double t = rayPlaneIntersect(normal, v0, rayOrigin, rayDirection);

        //Find the intersection point on the plane
        glm::vec3 P = rayOrigin + t * rayDirection;

        //Transform back to marker coordinates
        glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0, 0, 0, 1));
        P = glm::transpose(rotation) * glm::vec4(P.x, P.y, P.z, 1);
        std::cout<<P.x<<" " <<P.y<<" "<<P.z<<std::endl;
        intersectedText->setPos(P);
    } else {
        intersectedText->setPos(glm::vec3(xPosScreen, yPosScreen, 1));
    }
}

/**
 * Move the specified marker to the cursor position.
 * @param rayOrigin origin of the ray
 * @param rayDirection direction of the ray
 */
void GUI::moveMarker(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen) {
    glm::vec3 markerPos;
    if (intersectedMarkerNum == 1)
        markerPos = intersectedMarker->getMarker1Pos();
    else
        markerPos = intersectedMarker->getMarker2Pos();

    glm::vec3 v0 = modelWorld * (glm::vec4(markerPos.x, markerPos.y, markerPos.z, 1) * 10.0 - glm::vec4(5, 5, 5, 1));
    glm::vec4 normal = glm::vec4(0, 0, 1, 0);
    double t = rayPlaneIntersect(normal, v0, rayOrigin, rayDirection);

    //Find the intersection point on the plane
    glm::vec3 P = rayOrigin + t * rayDirection;

    //Find the snapping point on the plane
    if (snap)
        P = getSnapPoint(rayOrigin, P);

    //Transform back to marker coordinates
    glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0, 0, 0, 1));
    P = glm::transpose(rotation) * glm::vec4(P.x, P.y, P.z, 1);

    P = (P + glm::vec3(5, 5, 5)) / 10.0;

    if (P.x > 1) P.x = 1;
    if (P.x < 0) P.x = 0;
    if (P.y > 1) P.y = 1;
    if (P.y < 0) P.y = 0;
    if (P.z > 1) P.z = 1;
    if (P.z < 0) P.z = 0;

    // current marker position to render distance text later
    markerXPos = xPosScreen;
    markerYPos = yPosScreen;

    if (intersectedMarkerNum == 1)
        intersectedMarker->setPositionMarker1(P);
    else
        intersectedMarker->setPositionMarker2(P);

}

void GUI::moveMeasureObject(glm::vec3 rayOrigin, glm::vec3 rayDirection, float xPosScreen, float yPosScreen){
    glm::vec3 objPos = myObj.getPos();

    glm::vec3 v0 = modelWorld * (glm::vec4(objPos.x, objPos.y, objPos.z, 1));
    glm::vec4 normal = glm::vec4(0, 0, 1, 0);
    double t = rayPlaneIntersect(normal, v0, rayOrigin, rayDirection);

    //Find the intersection point on the plane
    glm::vec3 P = rayOrigin + t * rayDirection;

    //Transform back to marker coordinates
    glm::mat4 rotation = glm::mat4(modelWorld[0], modelWorld[1], modelWorld[2], glm::vec4(0, 0, 0, 1));
    P = glm::transpose(rotation) * glm::vec4(P.x, P.y, P.z, 1);

    float upperBound = 5.0f - *myObj.getSize();
    float lowerBound = -1 * upperBound;

    if (P.x > upperBound) P.x = upperBound;
    if (P.x < lowerBound) P.x = lowerBound;
    if (P.y > upperBound) P.y = upperBound;
    if (P.y < lowerBound) P.y = lowerBound;
    if (P.z > upperBound) P.z = upperBound;
    if (P.z < lowerBound) P.z = lowerBound;

    myObj.setPos(P);
}

/**
 * Calculates the value for t in the intersection between a plane and a ray
 * @param normal normal of the plane
 * @param point point on the plane
 * @param rayOrig ray origin (e)
 * @param rayDir ray direction (d)
 * @return the t value that satisfied e + td = P, where P is the intersection of the ray with the plane. -1 if no intersection
 */
double GUI::rayPlaneIntersect(glm::vec3 normal, glm::vec3 point, glm::vec3 rayOrig, glm::vec3 rayDir) {
    float denom = glm::dot(normal, rayDir);
    if (abs(denom) > 0.0001f) // your favorite epsilon
    {
        float t = glm::dot((point - rayOrig), normal) / denom;
        if (t >= 0) return t; // you might want to allow an epsilon here too
    }
    return -1;
}

void GUI::setWidth(int inWidth) {
    width = inWidth;
}

void GUI::setHeight(int inHeight) {
    height = inHeight;
}