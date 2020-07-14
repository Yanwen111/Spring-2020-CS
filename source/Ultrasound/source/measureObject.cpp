#include "measureObject.h"

MeasureObject::MeasureObject(){
//    pos = glm::vec3(
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    pos = glm::vec3(0,0,0);

    myColor = glm::vec3(1.0, 0.5, 0.5);
    mySize = 1.0;

    cubeSetUp();
}

void MeasureObject::cubeSetUp() {
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
            "   FragColor = vec4(result, 0.5);							 \n"
            "}										                     \n";

    cubeShader = Shader(vmarker.c_str(), fmarker.c_str(), false);

    // Add the marker
    cubeIndex = Helper::read_stl("config_file/models/cube.stl", cubevertices, cubenormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeNormalsVBO);
    glGenVertexArrays(1, &cubeVAO);

    glBindVertexArray(cubeVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeIndex * sizeof(GLfloat), cubevertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeIndex * sizeof(GLfloat), cubenormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
}

void scaleCube(glm::mat4& model_marker, float s){
    glm::vec3 scale(s, s, s);
    model_marker = glm::scale(glm::mat4(1.0f), scale) * model_marker;
}

void rotateCube(glm::mat4& model_marker, glm::mat4 modelRot){
    //Rotate to match the cube
    //add in orientation of marker
    model_marker = modelRot * model_marker;
    model_marker = glm::rotate(model_marker, glm::radians(90.0f), glm::vec3(-1, 0, 0));
}

void translateCube(glm::mat4& model_marker, glm::mat4 modelRot, glm::vec3 position){
    glm::vec3 trans = modelRot * glm::vec4(position,1.0f);
    model_marker = glm::translate(glm::mat4(1.0f), trans) * model_marker;
}

float MeasureObject::getSize(){
    return mySize;
}
void MeasureObject::setSize(float size){
    mySize = size;
}

glm::vec3 MeasureObject::getPos() {
    return pos;
}

void MeasureObject::setPos(glm::vec3 inPos) {
    pos = inPos;
}

/**
 * Draws a marker on the screen
 * @param projection the projection matrix
 * @param view the view matrix
 * @param model_marker the model matrix of the marker
 */
void MeasureObject::drawCube(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker){
    // Drawing the marker
    cubeShader.use();
    cubeShader.setMat4("projection", projection);
    cubeShader.setMat4("view", view);
    cubeShader.setMat4("model", model_marker);

    // Set lights
    if(isIntersected){
        cubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        cubeShader.setVec3("objectColor", myColor*1.5f);
    }
    else {
        cubeShader.setVec3("lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
        cubeShader.setVec3("objectColor", myColor);
    }
    cubeShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, cubeIndex/3);
}

void MeasureObject::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
    glEnable(GL_DEPTH_TEST);

    //draw square only if text is in 3D space
    model_cube = glm::mat4(1.0f);
    scaleCube(model_cube, mySize);
    rotateCube(model_cube, model);
    translateCube(model_cube, model, pos);
    drawCube(projection, view, model_cube);

    glDisable(GL_DEPTH_TEST);
}

void MeasureObject::setIntersected(bool value){
    isIntersected = value;
}

bool MeasureObject::getIntersected(){
    return isIntersected;
}


/**
 * Checks whether the ray created from the mouse intersects with text
 * @param rayOrigin
 * @param rayDirection
 * @param t where on the ray the marker is located
 * @return -1 for no intersection, 1 for marker1, 2 for marker2
 */
int MeasureObject::checkMouseOnCube(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t){
    int intersected = -1;

    //check if ray intersects the cube

    //loop over each triangle
    for (int x = 0; x < cubeIndex / 9; x++) {
        glm::vec4 v0 = glm::vec4(cubevertices[9 * x], cubevertices[9 * x + 1], cubevertices[9 * x + 2], 1);
        glm::vec4 v1 = glm::vec4(cubevertices[9 * x + 3], cubevertices[9 * x + 4], cubevertices[9 * x + 5],
                                 1);
        glm::vec4 v2 = glm::vec4(cubevertices[9 * x + 6], cubevertices[9 * x + 7], cubevertices[9 * x + 8],
                                 1);

        if (rayTriangleIntersect(rayOrigin, rayDirection, model_cube * v0, model_cube * v1, model_cube * v2,
                                 t))
            intersected = 1;

        if (intersected != -1)
            break;
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
bool MeasureObject::rayTriangleIntersect(
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
