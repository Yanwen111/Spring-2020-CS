#include <boost/filesystem/operations.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "text.h"

const int FONT_SIZE = 18;

Text::Text(const std::string inText, int scr_width_in, int scr_height_in, bool is3DText){
    is3D = is3DText;
    origText = inText;

    std::string delimiter = "\n";

    size_t last = 0;
    size_t next = 0;
    int numLines = 1;
    while ((next = inText.find(delimiter, last)) != std::string::npos) {
        text.push_back(inText.substr(last, next-last));
        last = next + 1;
        numLines += 1;
    }
    text.push_back(inText.substr(last));

    width = 0;
    height = FONT_SIZE * numLines;

    scr_width = scr_width_in;
    scr_height = scr_height_in;

    setUpFont();

    pos = glm::vec3(0,0,0);
    pRaster = glm::vec2(scr_width_in / 2, scr_height_in / 2);

    myColor = glm::vec3(1.0, 0.5, 0.5);
    mySize = 1.0;

    markerSetUp();
}

void Text::markerSetUp() {
    std::string vmarker =
            "// VERTEX SHADER											  \n"
            "															  \n"
            "#version 330 core											  \n"
            "															  \n"
            "layout (location = 0) in vec3 aPos;                          \n"
            "layout (location = 1) in vec3 aNormal;                       \n"
            "															  \n"
            "uniform mat4 projection;                                     \n"
            "uniform mat4 view;                                           \n"
            "uniform mat4 model;                                          \n"
            "                                                             \n"
            "out vec3 Normal;                                             \n"
            "out vec3 FragPos;                                            \n"
            "                                                             \n"
            "void main() {                                                \n"
            "   vec4 mvPos = view * model * vec4(aPos, 1.0);              \n"
            "   gl_Position = projection * mvPos;                         \n"
            "   Normal = vec3(view * model * vec4(aNormal, 0.0f));        \n"
            "   FragPos = vec3(view * model * vec4(aPos, 1.0f));          \n"
            "}";

    std::string fmarker =
            "// FRAGMENT SHADER											 \n"
            "															 \n"
            "#version 330 core											 \n"
            "															 \n"
            "out vec4 FragColor;										 \n"
            "                   										 \n"
            "in vec3 Normal;    										 \n"
            "in vec3 FragPos;	    									 \n"
            "				                    						 \n"
            "uniform vec3 lightPos;										 \n"
            "uniform vec3 lightColor;									 \n"
            "uniform vec3 objectColor;									 \n"
            "                   										 \n"
            "void main() {										         \n"
            "										                     \n"
            "    vec3 ambient = 0.3f * lightColor;						 \n"
            "    vec3 norm = normalize(Normal);						     \n"
            "    vec3 lightDir = normalize(lightPos - FragPos);			 \n"
            "    float diff = max(dot(norm, lightDir), 0.0);			 \n"
            "    vec3 diffuse = diff * lightColor;						 \n"
            "                                                            \n"
            "   vec3 result = (ambient + diffuse) * objectColor;		 \n"
            "   FragColor = vec4(result, 1.0);							 \n"
            "}										                     \n";

    markerShader = Shader(vmarker.c_str(), fmarker.c_str(), false);

    // Add the marker
    markerIndex = Helper::read_stl("config_file/models/cube.stl", markervertices, markernormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &markerVBO);
    glGenBuffers(1, &markerNormalsVBO);
    glGenVertexArrays(1, &markerVAO);

    glBindVertexArray(markerVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
    glBufferData(GL_ARRAY_BUFFER, markerIndex * sizeof(GLfloat), markervertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, markerNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, markerIndex * sizeof(GLfloat), markernormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
}

void scaleMarker(glm::mat4& model_marker, float s){
    glm::vec3 scale(s, s, s);
    model_marker = glm::scale(glm::mat4(1.0f), scale) * model_marker;
}

void rotateMarker(glm::mat4& model_marker, glm::mat4 modelRot){
    //Rotate to match the cube
    //add in orientation of marker
    model_marker = modelRot * model_marker;
    model_marker = glm::rotate(model_marker, glm::radians(90.0f), glm::vec3(-1, 0, 0));
}

void translateMarker(glm::mat4& model_marker, glm::mat4 modelRot, glm::vec3 position){
    glm::vec3 trans = modelRot * glm::vec4(position,1.0f);
    model_marker = glm::translate(glm::mat4(1.0f), trans) * model_marker;
}

/**
 * Draws a marker on the screen
 * @param projection the projection matrix
 * @param view the view matrix
 * @param model_marker the model matrix of the marker
 */
void Text::drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker){
    // Drawing the marker
    markerShader.use();
    markerShader.setMat4("projection", projection);
    markerShader.setMat4("view", view);
    markerShader.setMat4("model", model_marker);

    // Set lights
    if(isIntersected){
        markerShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        markerShader.setVec3("objectColor", myColor*1.5f);
    }
    else {
        markerShader.setVec3("lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
        markerShader.setVec3("objectColor", myColor);
    }
    markerShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(markerVAO);
    glDrawArrays(GL_TRIANGLES, 0, markerIndex/3);
}

bool computePixelCoordinates2(
        const glm::vec3 &pWorld,
        glm::mat4 projection,
        glm::mat4 view,
        glm::mat4 model,
        const int &imageWidth,
        const int &imageHeight,
        glm::vec2 &pRaster)
{
    //Convert to NDC coordinates: [-1, 1]
    glm::vec4 pNDC = projection * view * model * glm::vec4(pWorld.x, pWorld.y, pWorld.z, 1);

    //Convert NDC coordinates to screen coordinates
    pRaster.x = (pNDC.x/pNDC.z + 1)/2.0 * imageWidth;
    pRaster.y = (pNDC.y/pNDC.z + 1)/2.0 * imageHeight;

    return true;
}

glm::vec3 Text::getPos(){
    return pos;
}

bool Text::getIs3D() {
    return is3D;
}
void Text::set3D(bool value) {
    is3D = value;
}

float Text::getRasterX() {
    return pRaster.x;
}
float Text::getRasterY() {
    return pRaster.y;
}

void Text::setPos(glm::vec3 inPos) {
    if(is3D) {
        if (inPos.x > 5) inPos.x = 5;
        if (inPos.x < -5) inPos.x = -5;
        if (inPos.y > 5) inPos.y = 5;
        if (inPos.y < -5) inPos.y = -5;
        if (inPos.z > 5) inPos.z = 5;
        if (inPos.z < -5) inPos.z = -5;

        pos = inPos;
    } else {
        pRaster.x = inPos.x;
        pRaster.y = inPos.y;
//        std::cout<<"CHANGE: "<<pRaster.x<<" "<<pRaster.y<<std::endl;
    }
}

void Text::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    glEnable(GL_DEPTH_TEST);

    if(is3D){
        //draw square only if text is in 3D space
        model_marker = glm::mat4(1.0f);
        scaleMarker(model_marker, 0.1);
        rotateMarker(model_marker, model);
        translateMarker(model_marker, model, pos);
        drawMarker(projection, view, model_marker);

        computePixelCoordinates2(pos, projection, view, model, scr_width, scr_height, pRaster);
    }


    int lineNum = 0;
    bottomLeft = glm::vec2(pRaster.x, pRaster.y + FONT_SIZE);
    for(std::string txt: text){
        float lineWidth;
        if(isIntersected)
            lineWidth = RenderText(txt, pRaster.x, pRaster.y - lineNum*FONT_SIZE, mySize, myColor*1.5f);
        else
            lineWidth = RenderText(txt, pRaster.x, pRaster.y - lineNum*FONT_SIZE, mySize, myColor);
        if(lineWidth > width)
            width = lineWidth;
        lineNum++;
    }
    topRight = glm::vec2(pRaster.x + width, pRaster.y - (lineNum-1)*FONT_SIZE);

    glDisable(GL_DEPTH_TEST);
}

std::string Text::getText() {
    return origText;
}

void Text::setIntersected(bool value){
    isIntersected = value;
}

void Text::setUpFont() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    boost::filesystem::path filePath = boost::filesystem::current_path();
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
float Text::RenderText(std::string text, float x, float y, float scaleIn, glm::vec3 color) {
    // activate corresponding render state
    textShader.use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(scr_width), 0.0f, static_cast<float>(scr_height));
    textShader.setMat4("projection", projection);
    textShader.setVec3("textColor", color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    //create 5 pixel offset between marker and text
    x = x + 5;

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
        // update content of VBO mery
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices),
                        vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scaleIn; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return x;
}

/**
 * Checks whether the ray created from the mouse intersects with text
 * @param rayOrigin
 * @param rayDirection
 * @param t where on the ray the marker is located
 * @return -1 for no intersection, 1 for marker1, 2 for marker2
 */
int Text::checkMouseOnText(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t, float xPos, float yPos){
    int intersected = -1;

    //check if ray intersects the text marker
    if(is3D) {
        //loop over each triangle
        for (int x = 0; x < markerIndex / 9; x++) {
            glm::vec4 v0 = glm::vec4(markervertices[9 * x], markervertices[9 * x + 1], markervertices[9 * x + 2], 1);
            glm::vec4 v1 = glm::vec4(markervertices[9 * x + 3], markervertices[9 * x + 4], markervertices[9 * x + 5],
                                     1);
            glm::vec4 v2 = glm::vec4(markervertices[9 * x + 6], markervertices[9 * x + 7], markervertices[9 * x + 8],
                                     1);

            if (rayTriangleIntersect(rayOrigin, rayDirection, model_marker * v0, model_marker * v1, model_marker * v2,
                                     t))
                intersected = 1;

            if (intersected != -1){
                break;
            }
        }
    }
    else {
        //check if ray intersects the text
        if(xPos < topRight.x && xPos > bottomLeft.x && yPos > topRight.y && yPos < bottomLeft.y){
            t = 0;
            intersected = 1;
        }
    }
    return intersected;
}

/**
 * Checks whether a ray intersects a triangle with vertices v0, v1, v2
 * @param orig ray origin
 * @param dir ray direction
 * @param v0 vertex 0 of the triangle
 * @param v1 vertex 1 of the triangle
 * @param v2 vertex 2 of the triangle
 * @param t this method sets the value of t to be the point V = O + tD, when the ray intersects with the triangle.
 * Let P = the point of intersection, O = ray origin, D = ray direction.
 * @return true if it intersects, false otherwise.
 */
bool Text::rayTriangleIntersect(
        const glm::vec3 &orig, const glm::vec3 &dir,
        const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
        float &t)
{
    ///Barycentric coordinate calculation

    float a = v0.x-v1.x;
    float b = v0.y-v1.y;
    float c = v0.z-v1.z;
    float d = v0.x-v2.x;
    float e = v0.y-v2.y;
    float f = v0.z-v2.z;
    float g = dir.x;
    float h = dir.y;
    float i = dir.z;
    float j = v0.x-orig.x;
    float k = v0.y-orig.y;
    float l = v0.z-orig.z;

    float M = a*(e*i-h*f)+b*(g*f-d*i)+c*(d*h-e*g);

    t = -1*(f*(a*k-j*b) + e*(j*c-a*l) + d*(b*l-k*c))/M;
    if(t < 0.000001) return false;

    float gamma = (i*(a*k-j*b)+h*(j*c-a*l)+g*(b*l-k*c))/M;
    if( gamma < 0 || gamma > 1) return false;

    float beta = (j*(e*i-h*f)+k*(g*f-d*i)+l*(d*h-e*g))/M;
    if(beta < 0 || beta > 1-gamma) return false;

    return true;
}
