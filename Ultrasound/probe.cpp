#define _USE_MATH_DEFINES
#include <cmath>
#include "probe.h"

int read_stl(const std::string& fname, GLfloat * &vertices, GLfloat * &colors);

Probe::Probe(std::string filename){
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
            "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
            "   Normal = aNormal;                                         \n"
            "   FragPos = vec3(model * vec4(aPos, 1.0f));                 \n"
            "}												              \n";

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
    probeindex = read_stl(filename, probevertices, probeNormals);

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
}

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
    probeShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 15.0f));

    glBindVertexArray(probeVAO);
    glDrawArrays(GL_TRIANGLES, 0, probeindex/3);

    glDisable(GL_DEPTH_TEST);
}

void Probe::openIMUFile(const std::string& inputFileName){
    file.open(inputFileName);
}

void Probe::closeFile(){
    file.close();
}

glm::vec4 Probe::readNextLine(){
    std::string line;
    glm::vec4 quat;
    if (getline(file, line))
    {
        quat = parseLine(&line[0]);
    }
    return quat;
}

glm::vec4 Probe::parseLine(char str[])
{
    char *token = std::strtok(str, " ");
    float w = std::stof(token);

    token = strtok(NULL, " ");
    float x = std::stof(token);

    token = strtok(NULL, " ");
    float y = std::stof(token);

    token = strtok(NULL, " ");
    float z = std::stof(token);

    return glm::vec4(w, x, y, z);
}


glm::mat4 Probe::getOrientation(){
    glm::vec4 quat = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    try{
        quat = readNextLine();
        //printf("The readed quat are: w = %f, x = %f, y = %f, z = %f\n", quat.x, quat.y, quat.z, quat.w);
    }
    catch (int e){
        if (file.is_open()) {
            std::cout << "Closing file" << std::endl;
            closeFile();
        }
    }

    glm::mat4 rotationMatrix = Rotation::convertRotationMatrix(quat.x, quat.y, quat.z, quat.w);

    return rotationMatrix;
}

int read_stl(const std::string& file_name, GLfloat * &vertices, GLfloat * &normals)
{

    std::ifstream myfile (file_name.c_str(), std::ios::in | std::ios::binary);

    char header_info[80] = "";
    char bin_n_triangles[4];
    unsigned int num_traingles = 0;

    if (myfile)
    {
        myfile.read (header_info, 80);
        std::cout <<"Header : " << header_info << std::endl;
    }

    if (myfile)
    {
        myfile.read (bin_n_triangles, 4);
        num_traingles = *((unsigned int*)bin_n_triangles) ;
        std::cout <<"Number of triangles : " << num_traingles << std::endl;
    }

    vertices = new GLfloat[num_traingles * 9];
    normals = new GLfloat[num_traingles * 9];

    int index = 0;
    int indexN = 0;
    for(int i = 0; i < num_traingles; i++)
    {
        char facet[50];
        if (myfile)
        {
            myfile.read (facet, 50);

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            vertices[index++] = *( (float*) ( ( (char*)facet)+12));
            vertices[index++] = *( (float*) ( ( (char*)facet)+16));
            vertices[index++] = *( (float*) ( ( (char*)facet)+20));

            vertices[index++] = *( (float*) ( ( (char*)facet)+24));
            vertices[index++] = *( (float*) ( ( (char*)facet)+28));
            vertices[index++] = *( (float*) ( ( (char*)facet)+32));

            vertices[index++] = *( (float*) ( ( (char*)facet)+36));
            vertices[index++] = *( (float*) ( ( (char*)facet)+40));
            vertices[index++] = *( (float*) ( ( (char*)facet)+44));
        }
    }
    return index;
}