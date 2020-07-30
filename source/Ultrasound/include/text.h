#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <map>
#include "shader.h"
#include "helper.h"

class Text {
public:
//    Text();
    Text(std::string inText, int scr_width, int scr_height, bool is3D);

    glm::vec3 getPos();
    void setPos(glm::vec3 inPos);

    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

    std::string getText();

    int checkMouseOnText(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t, float xPos, float yPos);

    void setIntersected(bool value);

    bool getIs3D();
    void set3D(bool value);

    float getRasterX();
    float getRasterY();

private:
    glm::vec3 pos;
//    std::string text;
    std::vector<std::string> text;

    //whether or not the text moves in 3D space
    bool is3D = false;

    //where the text starts and ends
    glm::vec2 bottomLeft;
    glm::vec2 topRight;

    float width;
    float height;

    std::string origText;

    bool isIntersected = false;

    //location of the text on the screen
    glm::vec2 pRaster;

    int scr_width;
    int scr_height;

    float mySize;
    glm::vec3 myColor;

    void setUpFont();

    //****************************  Render 2D Text Vars ************************************
    //Characters map needed for FreeType
    struct Character {
        unsigned int TextureID;  // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        unsigned int Advance;    // Offset to advance to next glyph
    };
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;

    //returns the ending x position of the rendered text
    float RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

    Shader textShader;

    //vertices and normals of the marker in STL file
    GLfloat *markervertices = NULL;
    GLfloat *markernormals = NULL;

    Shader markerShader;
    unsigned int markerVAO, markerVBO, markerNormalsVBO;
    //number of vertices in the STL file
    int markerIndex;
    void drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker);
    glm::mat4 model_marker;

    void markerSetUp();
    bool rayTriangleIntersect(
            const glm::vec3 &orig, const glm::vec3 &dir,
            const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
            float &t);
};
