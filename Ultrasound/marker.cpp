#include "marker.h"

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
            "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
            "   Normal = aNormal;                                         \n"
            "   FragPos = vec3(model * vec4(aPos, 1.0f));                 \n"
            "}												              \n";

    std::string fmarker =
            "// FRAGMENT SHADER											 \n"
            "															 \n"
            "#version 330 core											 \n"
            "															 \n"
            "out vec4 FragColor;										 \n"
            "                   										 \n"
            "in vec3 Normal;    										 \n"
            "in vec3 FragPos;	    									 \n"
            "                   										 \n"
            "void main() {										         \n"
            "										                     \n"
            "   FragColor = vec4(1.0, 0., 1.0, 1.0);					 \n"
            "}										                     \n";

    // Read the markers
    markerIndex = Helper::read_stl("probe.stl", markervertices, markernormals);
    markerShader = Shader(vmarker.c_str(), fmarker.c_str(), false);
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
}

void Marker::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
//    glEnable(GL_DEPTH_TEST);

    glm::mat4 model_probe      = glm::mat4(1.0f);

    //Scale probe
    glm::vec3 probeScale(1000.f, 1000.f, 1000.f);
    model_probe      = glm::scale(glm::mat4(1.0f), probeScale) * model_probe;

    markerShader.use();
    markerShader.setMat4("projection", projection);
    markerShader.setMat4("view", view);
    markerShader.setMat4("model", model_probe);

    glBindVertexArray(markerVAO);
    glDrawArrays(GL_TRIANGLES, 0, markerIndex/3);

//    glDisable(GL_DEPTH_TEST);
}
