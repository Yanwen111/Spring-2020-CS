//
// Created by Hayun Chong on 3/24/20.
//
#include "scale.h"

static const GLfloat cubeVertexData[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
};
static const std::string vmarker =
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

static const std::string fmarker =
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
        "uniform float transparent;									 \n"
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
        "   FragColor = vec4(result, transparent);							 \n"
        "}										                     \n";



static const float LETTER_SIZE = 0.25f;
static const glm::vec4 MARKER_COLOR = glm::vec4(0.2f, 0.5f, 0.5f, 1.0f);
static const glm::vec4 TICK_COLOR = glm::vec4(0.15f, 0.65f, 0.55f, 1.0f);
static const glm::vec3 NUMBER_COLOR = glm::vec3(0.9, 0.5, 0.5);

Scale::Scale(){
    // Add the 2x2x2 cube centered at origin
    cubeIndex = Helper::read_stl("config_file/models/cube_2_2.stl", cubevertices, cubenormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &scaleVBO);
    glGenBuffers(1, &scaleNormalsVBO);
    glGenVertexArrays(1, &scaleVAO);

    glBindVertexArray(scaleVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeIndex * sizeof(GLfloat), cubevertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, scaleNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeIndex * sizeof(GLfloat), cubenormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Clear unused memory
    delete [] cubevertices;
    delete [] cubenormals;

    scaleShader = Shader(vmarker.c_str(), fmarker.c_str(), false);

    std::string numberFiles[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

    numberVertices.assign(10, NULL);
    numberNormals.assign(10, NULL);

    //Set up Numbers
    for(int x = 0; x < 10; x++){
        numberIndex.push_back(Helper::read_stl("config_file/models/"+numberFiles[x]+".stl", numberVertices.at(x), numberNormals.at(x)));

        //Set up OpenGL buffers
        glGenBuffers(1, &numberVBO[x]);
        glGenBuffers(1, &numberNormalsVBO[x]);
        glGenVertexArrays(1, &numberVAO[x]);

        glBindVertexArray(numberVAO[x]);

        //position attribute
        glBindBuffer(GL_ARRAY_BUFFER, numberVBO[x]);
        glBufferData(GL_ARRAY_BUFFER, numberIndex.at(x) * sizeof(GLfloat), numberVertices.at(x), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        //normals attribute
        glBindBuffer(GL_ARRAY_BUFFER, numberNormalsVBO[x]);
        glBufferData(GL_ARRAY_BUFFER, numberIndex.at(x) * sizeof(GLfloat), numberNormals.at(x), GL_STATIC_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        delete [] numberVertices.at(x);
        delete [] numberNormals.at(x);
    }
}

void Scale::setMeasurements(double freqIn, double velIn, int depthIn){
    freq = freqIn;
    vel = velIn;
    depth = depthIn;

    // cm of one side
    double sideLength = ((1/freq)*vel*depth/2.0f / 10000.0);

    //Each side length is 10 in the cube. Length of each full mark
    double markLength = 10.0 / sideLength;

    int numMarks = (int)sideLength;

    //Insert location of every 1/2 cm to linesPlacement
    linesPlacement.clear();
    double location = 0.0f;
    while(location <= 10){
        linesPlacement.push_back(location);
        location = location + markLength/2.0f;
    }
}


void Scale::scaleObj(glm::mat4& model_marker, float x, float y, float z){
    glm::vec3 scale(x, y, z);
    model_marker = glm::scale(glm::mat4(1.0f), scale) * model_marker;
}

void Scale::rotate(glm::mat4& model_marker, glm::mat4 modelRot){
    //Rotate to match the cube
    //add in orientation of marker
    model_marker = modelRot * model_marker;
    model_marker = glm::rotate(model_marker, glm::radians(90.0f), glm::vec3(-1, 0, 0));
}

void Scale::translate(glm::mat4& model_marker, glm::mat4 modelRot, glm::vec3 position){
    glm::vec3 trans = modelRot * glm::vec4(position,1.0f);
    model_marker = glm::translate(glm::mat4(1.0f), trans) * model_marker;
}

void Scale::showGrid(bool value) {
    gridShown = value;
}

bool Scale::isGridShown() {
    return gridShown;
}

float gridSize = 0.015;
glm::vec4 GRID_COLOR = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);

void Scale::drawCross(glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec3 loc) {
    //Draw the vertical lines
    glm::mat4 model_grid = glm::mat4(1.0f);
    scaleObj(model_grid, gridSize, 0.2, gridSize);
    rotate(model_grid, model);
    translate(model_grid, model, loc);
    drawCubeShader(projection, view, model_grid, GRID_COLOR);

    //Draw the vertical lines
    model_grid = glm::mat4(1.0f);
    scaleObj(model_grid, 0.2, gridSize, gridSize);
    rotate(model_grid, model);
    translate(model_grid, model, loc);
    drawCubeShader(projection, view, model_grid, GRID_COLOR);

    //Draw the vertical lines
    model_grid = glm::mat4(1.0f);
    scaleObj(model_grid, gridSize, gridSize,0.2);
    rotate(model_grid, model);
    translate(model_grid, model, loc);
    drawCubeShader(projection, view, model_grid, GRID_COLOR);
}

void Scale::drawNumber(glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec3 loc, int first, int second, float letter_size, glm::vec3 number_color) {
    //Draw 10's digit
    if(second > 0){
        glm::mat4 model_num = glm::mat4(1.0f);
        scaleObj(model_num, letter_size, letter_size, letter_size);
        translate(model_num, model, loc - glm::vec3(letter_size/2.0, 0,0));
        drawNumberShader(projection, view, model_num, number_color, second);
    }
    //Draw first digit
    glm::mat4 model_num = glm::mat4(1.0f);
    scaleObj(model_num, letter_size, letter_size, letter_size);
    translate(model_num, model, loc);
    drawNumberShader(projection, view, model_num, number_color, first);
}

void Scale::drawGrid(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {

    //Crossed Version
    for(int x = 0; x < linesPlacement.size()/2; x+=2) {
        for(int y = 0; y < linesPlacement.size(); y+=2) {
            for(int z = 0; z < linesPlacement.size()/2; z+=2) {
                drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), 5 - linesPlacement.at(y), -1*linesPlacement.at(z)));
                drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), 5 - linesPlacement.at(y), linesPlacement.at(z)));
                drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), 5 - linesPlacement.at(y), linesPlacement.at(z)));
                drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), 5 - linesPlacement.at(y), -1*linesPlacement.at(z)));
            }
        }
    }

    float letter_size = 0.25;
    glm::vec3 number_color = glm::vec3(1.0,1.0,1.0);

    //x-axis markers
    {
        int y = 5;
        int z = -5;
        int first = 0;
        int second = 0;
        for (int x = 0; x < linesPlacement.size()/2; x += 2) {

            //top back x-axis
            drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), y, z));
            drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), y, z));

            drawNumber(projection, view, model, glm::vec3(linesPlacement.at(x), y+0.25, z-0.25), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(-1*linesPlacement.at(x), y+0.25, z-0.25), first, second, letter_size, number_color);

            //top front x-axis
            drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), 5, 5));
            drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), 5, 5));

            drawNumber(projection, view, model, glm::vec3(linesPlacement.at(x), 5+0.25, 5+0.25), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(-1*linesPlacement.at(x), 5+0.25, 5+0.25), first, second, letter_size, number_color);

            //bottom back x-axis
            drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), -5, -5));
            drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), -5, -5));

            drawNumber(projection, view, model, glm::vec3(linesPlacement.at(x), -5-0.25, -5-0.25), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(-1*linesPlacement.at(x), -5-0.25, -5-0.25), first, second, letter_size, number_color);

            //bottom front x-axis
            drawCross(projection, view, model, glm::vec3(linesPlacement.at(x), -5, 5));
            drawCross(projection, view, model, glm::vec3(-1*linesPlacement.at(x), -5, 5));

            drawNumber(projection, view, model, glm::vec3(linesPlacement.at(x), -5-0.25, 5+0.25), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(-1*linesPlacement.at(x), -5-0.25, 5+0.25), first, second, letter_size, number_color);

            first++;
            if (first == 10) {
                first = 0;
                second += 1;
            }
        }
    }
    //y-axis markers
    {
        int first = 0;
        int second = 0;
        for (int y = 0; y < linesPlacement.size(); y += 2) {
            int x = -5;
            int z = -5;
            //left back y-axis
            drawCross(projection, view, model, glm::vec3(x, 5 - linesPlacement.at(y), z));
            drawNumber(projection, view, model, glm::vec3(x - 0.25, 5 - linesPlacement.at(y), z - 0.25), first, second, letter_size, number_color);

            //right back y-axis
            x=5, z=-5;
            drawCross(projection, view, model, glm::vec3(x, 5 - linesPlacement.at(y), z));
            drawNumber(projection, view, model, glm::vec3(x + 0.25, 5 - linesPlacement.at(y), z - 0.25), first, second, letter_size, number_color);

            //right front y-axis
            x=5, z=5;
            drawCross(projection, view, model, glm::vec3(x, 5 - linesPlacement.at(y), z));
            drawNumber(projection, view, model, glm::vec3(x + 0.25, 5 - linesPlacement.at(y), z + 0.25), first, second, letter_size, number_color);

            //left front y-axis
            x=-5, z=5;
            drawCross(projection, view, model, glm::vec3(x, 5 - linesPlacement.at(y), z));
            drawNumber(projection, view, model, glm::vec3(x - 0.25, 5 - linesPlacement.at(y), z + 0.25), first, second, letter_size, number_color);

            first++;
            if (first == 10) {
                first = 0;
                second += 1;
            }
        }
    }
     //z-axis markers
    {
        int first = 0;
        int second = 0;
        for (int z = 0; z < linesPlacement.size()/2; z += 2) {

            //left bottom z-axis
            int x = -5;
            int y = -5;
            drawCross(projection, view, model, glm::vec3(x, y, -1*linesPlacement.at(z)));
            drawCross(projection, view, model, glm::vec3(x, y, linesPlacement.at(z)));

            drawNumber(projection, view, model, glm::vec3(x-0.25, y-0.25, -1*linesPlacement.at(z)), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(x-0.25, y-0.25, linesPlacement.at(z)), first, second, letter_size, number_color);

            //left top z-axis
            x=-5, y=5;
            drawCross(projection, view, model, glm::vec3(x, y, -1*linesPlacement.at(z)));
            drawCross(projection, view, model, glm::vec3(x, y, linesPlacement.at(z)));

            drawNumber(projection, view, model, glm::vec3(x-0.25, y+0.25, -1*linesPlacement.at(z)), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(x-0.25, y+0.25, linesPlacement.at(z)), first, second, letter_size, number_color);

            //right top z-axis
            x=5,y=5;
            drawCross(projection, view, model, glm::vec3(x, y, -1*linesPlacement.at(z)));
            drawCross(projection, view, model, glm::vec3(x, y, linesPlacement.at(z)));

            drawNumber(projection, view, model, glm::vec3(x+0.25, y+0.25, -1*linesPlacement.at(z)), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(x+0.25, y+0.25, linesPlacement.at(z)), first, second, letter_size, number_color);

            //right bottom z-axis
            x=5,y=-5;
            drawCross(projection, view, model, glm::vec3(x, y, -1*linesPlacement.at(z)));
            drawCross(projection, view, model, glm::vec3(x, y, linesPlacement.at(z)));

            drawNumber(projection, view, model, glm::vec3(x+0.25, y-0.25, -1*linesPlacement.at(z)), first, second, letter_size, number_color);
            drawNumber(projection, view, model, glm::vec3(x+0.25, y-0.25, linesPlacement.at(z)), first, second, letter_size, number_color);

            first++;
            if (first == 10) {
                first = 0;
                second += 1;
            }
        }
    }


}

void Scale::drawYScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model,
        double positionX, double positionZ, glm::vec4 marker_color = MARKER_COLOR, glm::vec4 tick_color = TICK_COLOR,
        glm::vec3 number_color = NUMBER_COLOR, float letter_size = LETTER_SIZE){

    glm::mat4 model_marker      = glm::mat4(1.0f);

    scaleObj(model_marker, 0.05, 5, 0.05);
    rotate(model_marker, model);
    translate(model_marker, model, glm::vec3(positionX, 0, positionZ));

    drawCubeShader(projection, view, model_marker, marker_color);

    int first = 0;
    int second = 0;
    for(int x = 0; x < linesPlacement.size(); x++){
        glm::mat4 model_marks      = glm::mat4(1.0f);
        scaleObj(model_marks, 0.15, 0.05, 0.15);
        //1/2 a cm mark then scale to 0.5 size of normal
        if(x%2 == 1){
            scaleObj(model_marks, 0.5, 0.5, 0.5);
        }
        rotate(model_marks, model);
        translate(model_marks, model, glm::vec3(positionX,  5-linesPlacement.at(x), positionZ));
        drawCubeShader(projection, view, model_marks, tick_color);

        //Skip writing the number if it is 1/2cm mark
        if(x%2 == 1) continue;

        drawNumber(projection, view, model, glm::vec3(positionX-0.25,  5-linesPlacement.at(x), positionZ+0.25), first, second, LETTER_SIZE, NUMBER_COLOR);

        first++;
        if(first == 10){
            first = 0;
            second += 1;
        }
    }
}

void Scale::drawXScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model,
        double positionY, double positionZ){
    glm::mat4 model_marker      = glm::mat4(1.0f);

    scaleObj(model_marker, 5, 0.05, 0.05);
    rotate(model_marker, model);
    translate(model_marker, model, glm::vec3(0, positionY, positionZ));

    drawCubeShader(projection, view, model_marker, MARKER_COLOR);

    int first = 0;
    int second = 0;
    for(int x = 0; x < linesPlacement.size()/2; x++){
        //right side of zero
        glm::mat4 model_marks      = glm::mat4(1.0f);
        scaleObj(model_marks, 0.05,0.15, 0.15);
        //1/2 a cm mark then scale to 0.5 size of normal
        if(x%2 == 1){
            scaleObj(model_marks, 0.5, 0.5, 0.5);
        }
        rotate(model_marks, model);
        translate(model_marks, model, glm::vec3(linesPlacement.at(x),  positionY, positionZ));
        drawCubeShader(projection, view, model_marks, TICK_COLOR);

        //left side of zero
        model_marks      = glm::mat4(1.0f);
        scaleObj(model_marks, 0.05,0.15, 0.15);
        //1/2 a cm mark then scale to 0.5 size of normal
        if(x%2 == 1){
            scaleObj(model_marks, 0.5, 0.5, 0.5);
        }
        rotate(model_marks, model);
        translate(model_marks, model, glm::vec3(-1*linesPlacement.at(x),  positionY, positionZ));
        drawCubeShader(projection, view, model_marks, TICK_COLOR);

        //Skip writing the number if it is 1/2cm mark
        if(x%2 == 1) continue;
        drawNumber(projection, view, model, glm::vec3(linesPlacement.at(x),  positionY+0.25, positionZ+0.25), first, second, LETTER_SIZE, NUMBER_COLOR);
        drawNumber(projection, view, model, glm::vec3(-1*linesPlacement.at(x),  positionY+0.25, positionZ+0.25), first, second, LETTER_SIZE, NUMBER_COLOR);

        first++;
        if(first == 10){
            first = 0;
            second += 1;
        }
    }

}

void Scale::drawZScale(glm::mat4 projection, glm::mat4 view, glm::mat4 model,
        double positionX, double positionY){

    glm::mat4 model_marker      = glm::mat4(1.0f);
    scaleObj(model_marker, 0.05, 0.05, 5);
    rotate(model_marker, model);
    translate(model_marker, model, glm::vec3(positionX, positionY, 0));

    drawCubeShader(projection, view, model_marker, MARKER_COLOR);

    int first = 0;
    int second = 0;
    for(int x = 0; x < linesPlacement.size()/2; x++){

        //front section
        glm::mat4 model_marks      = glm::mat4(1.0f);
        scaleObj(model_marks, 0.15, 0.15, 0.05);
        //1/2 a cm mark then scale to 0.5 size of normal
        if(x%2 == 1){
            scaleObj(model_marks, 0.5, 0.5, 0.5);
        }
        rotate(model_marks, model);
        translate(model_marks, model, glm::vec3(positionX,  positionY, linesPlacement.at(x)));
        drawCubeShader(projection, view, model_marks, TICK_COLOR);

        //back section
        model_marks      = glm::mat4(1.0f);
        scaleObj(model_marks, 0.15, 0.15, 0.05);
        //1/2 a cm mark then scale to 0.5 size of normal
        if(x%2 == 1){
            scaleObj(model_marks, 0.5, 0.5, 0.5);
        }
        rotate(model_marks, model);
        translate(model_marks, model, glm::vec3(positionX,  positionY, -1*linesPlacement.at(x)));
        drawCubeShader(projection, view, model_marks, TICK_COLOR);

        //Skip writing the number if it is 1/2cm mark
        if(x%2 == 1) continue;

        drawNumber(projection, view, model, glm::vec3(positionX-0.25, positionY+0.25, linesPlacement.at(x)), first, second, LETTER_SIZE, NUMBER_COLOR);
        drawNumber(projection, view, model, glm::vec3(positionX-0.25, positionY+0.25, -1*linesPlacement.at(x)), first, second, LETTER_SIZE, NUMBER_COLOR);

        first++;
        if(first == 10){
            first = 0;
            second += 1;
        }
    }
}

void Scale::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec2 yPos, glm::vec2 xPos, glm::vec2 zPos){
    glEnable(GL_DEPTH_TEST);

    drawXScale(projection, view, model, yPos.x*10-5, yPos.y*10-5);
    drawYScale(projection, view, model, xPos.x*10-5, xPos.y*10-5);
    drawZScale(projection, view, model, zPos.x*10-5, zPos.y*10-5);

    if(gridShown)
        drawGrid(projection, view, model);

    glDisable(GL_DEPTH_TEST);
}

void Scale::drawNumberShader(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, glm::vec3 objColor, int number){
    // Drawing the marker
    scaleShader.use();
    scaleShader.setMat4("projection", projection);
    scaleShader.setMat4("view", view);
    scaleShader.setMat4("model", model_scale);

    // Set lights
    scaleShader.setVec3("objectColor", objColor);
    scaleShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    scaleShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));
    scaleShader.setFloat("transparent", 1.0);

    glBindVertexArray(numberVAO[number]);
    glDrawArrays(GL_TRIANGLES, 0, numberIndex.at(number)/3);
}

void Scale::drawCubeShader(glm::mat4 projection, glm::mat4 view, glm::mat4 model_scale, glm::vec4 objColor){
    // Drawing the marker
    scaleShader.use();
    scaleShader.setMat4("projection", projection);
    scaleShader.setMat4("view", view);
    scaleShader.setMat4("model", model_scale);

    // Set lights
    scaleShader.setVec3("objectColor", objColor);
    scaleShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    scaleShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));
    scaleShader.setFloat("transparent", objColor.z);

    glBindVertexArray(scaleVAO);
    glDrawArrays(GL_TRIANGLES, 0, cubeIndex/3);
}