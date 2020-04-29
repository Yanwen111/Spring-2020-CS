#define _USE_MATH_DEFINES
#include <cmath>
#include "probe.h"

/**
 * The Probe class loads an STL file of the probe onto the screen with the correct orientation.
 * At each draw, it applies the rotations found onto the probe.
 */
Probe::Probe(){
}

/**
 * Loads a new probe and sets up OpenGL buffers
 * @param filename the STL file of the probe
 */
void Probe::loadNewProbe(std::string filename){
    std::string vProbe =
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
            //            "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
            //            "   Normal = vec3(view * model * vec4(aNormal, 0.0f));        \n"
            //            "   FragPos = vec3(model * vec4(aPos, 1.0f));                 \n"
            "   vec4 mvPos = view * model * vec4(aPos, 1.0);              \n"
            "   gl_Position = projection * mvPos;                         \n"
            "   Normal = vec3(view * model * vec4(aNormal, 0.0f));        \n"
            "   FragPos = vec3(view * model * vec4(aPos, 1.0f));          \n"
            "}";

    std::string fProbe =
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

    probeShader = Shader(vProbe.c_str(), fProbe.c_str(), false);

    // Add the 3D probe
    GLfloat *probevertices = NULL;
    GLfloat *probeNormals = NULL;
    probeindex = Helper::read_stl(filename, probevertices, probeNormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &probeVBO);
    glGenBuffers(1, &probeNormalsVBO);
    glGenVertexArrays(1, &probeVAO);

    glBindVertexArray(probeVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, probeVBO);
    glBufferData(GL_ARRAY_BUFFER, probeindex * sizeof(GLfloat), probevertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, probeNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, probeindex * sizeof(GLfloat), probeNormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Clear unused memory
    delete [] probevertices;
    delete [] probeNormals;

    file = static_cast<std::basic_ifstream<char, std::char_traits<char>>>(NULL);
}

/**
 * Draws the probe on the screen with the applied rotations
 * @param projection projection matrix
 * @param view view matrix
 * @param rotationX the rotationX of the grid
 * @param rotationY the rotationY of the grid
 */
void Probe::draw(glm::mat4 projection, glm::mat4 view, float rotationX, float rotationY){

    glEnable(GL_DEPTH_TEST);
    //Orient and scale the probe to the center
    glm::mat4 model_probe      = glm::mat4(1.0f);

    //Scale probe
    glm::vec3 probeScale(0.05f, 0.05f, 0.05f);
    model_probe      = glm::scale(glm::mat4(1.0f), probeScale) * model_probe;

    //Rotate to match the cube
    glm::mat4 cubeRotation = glm::mat4(1.0f);
    cubeRotation = glm::rotate(cubeRotation, rotationY, glm::vec3(0, 1, 0));
    cubeRotation = glm::rotate(cubeRotation, rotationX, glm::rotate(glm::vec3(1, 0, 0), rotationY, glm::vec3(0, -1, 0)));

    //add in orientation of probe
    model_probe = cubeRotation * getOrientation() * model_probe;

    model_probe = glm::rotate(model_probe, glm::radians(90.0f), glm::vec3(0, 0, 1));
    model_probe = glm::rotate(model_probe, glm::radians(90.0f), glm::vec3(1, 0, 0));

    //Translate probe to top of cube
    glm::vec3 up = glm::vec3(0,5,0);
    up = glm::rotate(up, rotationX, glm::rotate(glm::vec3(1, 0, 0), rotationY, glm::vec3(0, -1, 0)));
    model_probe = glm::translate(glm::mat4(1.0f), glm::vec3(0, up.y,0)) * model_probe;

    // Drawing the probe
    probeShader.use();
    probeShader.setMat4("projection", projection);
    probeShader.setMat4("view", view);
    probeShader.setMat4("model", model_probe);

    // Set lights
    probeShader.setVec3("objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
    probeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    probeShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));


    glBindVertexArray(probeVAO);
    glDrawArrays(GL_TRIANGLES, 0, probeindex/3);

    glDisable(GL_DEPTH_TEST);
}

/**
 * Opens an IMU file (the file to read the probe rotations from)
 * @param inputFileName filename of the IMU file
 */
void Probe::openIMUFile(const std::string& inputFileName){
    file.open(inputFileName);
}

/**
 * close the file
 */
void Probe::closeFile(){
    file.close();
}

/**
 * Reads the next line of the IMU file
 * @return the next rotation in quaternion rotations
 */
glm::vec4 Probe::readNextLine(){
    std::string line;
    glm::vec4 myQuat;
    if (getline(file, line))
    {
        myQuat = parseLine(&line[0]);
    }
    return myQuat;
}

glm::vec4 Probe::parseLine(char str[])
{
    glm::vec4 myQuat;
    char *token = std::strtok(str, " ");
    myQuat.w = std::stof(token);

    token = strtok(NULL, " ");
    myQuat.x = std::stof(token);

    token = strtok(NULL, " ");
    myQuat.y = std::stof(token);

    token = strtok(NULL, " ");
    myQuat.z = std::stof(token);

    return myQuat;
}

/**
 * Gets the rotation of the probe based on the rotation file
 * @return the rotation matrix
 */
glm::mat4 Probe::getOrientation(){
    try{
        quat = readNextLine();
    }
    catch (int e){
        if (file.is_open()) {
            std::cout << "Closing file" << std::endl;
            closeFile();
        }
    }

    glm::mat4 rotationMatrix = Rotation::convertRotationMatrix(quat.w, quat.x, quat.y, quat.z);
//    glm::mat4 rotationMatrix = Rotation::convertRotationMatrix(quat.x, quat.y, quat.z, quat.w);

    return rotationMatrix;
}

glm::vec4 Probe::getQuaternions(){
    return quat;
}

glm::vec3 Probe::getEulerAngles() {
//    Rotation::convertToEulerAngle(quat.x, quat.y, quat.z, quat.w);
    return Rotation::convertToEulerAngle(quat.w, quat.x, quat.y, quat.z);
}
