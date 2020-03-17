#include "marker.h"

void scale(glm::mat4& model, float s);
void rotate(glm::mat4& model_marker, glm::mat4 modelRot);

Marker::Marker(){
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

    // Add the 3D probe
    markerIndex = Helper::read_stl("data/marker.stl", markervertices, markernormals);

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

    // Clear unused memory
    delete [] markervertices;
    delete [] markernormals;

    marker1 = glm::vec3(4,0,0);
    marker2 = glm::vec3(-4,0,0);
}

void scale(glm::mat4& model_marker, float s){
    glm::vec3 scale(s, s, s);
    model_marker = glm::scale(glm::mat4(1.0f), scale) * model_marker;
}

void rotate(glm::mat4& model_marker, glm::mat4 modelRot){
    //Rotate to match the cube
    //add in orientation of marker
    model_marker = modelRot * model_marker;
    model_marker = glm::rotate(model_marker, glm::radians(90.0f), glm::vec3(-1, 0, 0));
}

void translate(glm::mat4& model_marker, glm::mat4 modelRot, glm::vec3 position){
    glm::vec3 trans = modelRot * glm::vec4(position,1.0f);
    model_marker = glm::translate(glm::mat4(1.0f), trans) * model_marker;
}

void Marker::drawMarker(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker){
    // Drawing the marker
    markerShader.use();
    markerShader.setMat4("projection", projection);
    markerShader.setMat4("view", view);
    markerShader.setMat4("model", model_marker);

    // Set lights
    markerShader.setVec3("objectColor", glm::vec3(1.0f, 0.0f, 0.8f));
    markerShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    markerShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(markerVAO);
    glDrawArrays(GL_TRIANGLES, 0, markerIndex/3);
}

void Marker::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    glEnable(GL_DEPTH_TEST);

    //Draw Marker 1
    glm::mat4 model_marker      = glm::mat4(1.0f);

    scale(model_marker, 0.5f);
    rotate(model_marker, model);
    translate(model_marker, model, marker1);

    drawMarker(projection, view, model_marker);

    //Draw Marker 2
    model_marker      = glm::mat4(1.0f);

    scale(model_marker, 0.5f);
    rotate(model_marker, model);
    translate(model_marker, model, marker2);

    drawMarker(projection, view, model_marker);

    glDisable(GL_DEPTH_TEST);
}

float Marker::getDistance(){
    return length(marker2 - marker1);
}
