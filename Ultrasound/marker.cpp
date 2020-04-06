#include "marker.h"

void scale(glm::mat4& model, float s);
void rotate(glm::mat4& model_marker, glm::mat4 modelRot);

bool rayTriangleIntersect(
        const glm::vec3 &orig, const glm::vec3 &dir,
        const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
        float &t);

static const double MARKER_SIZE = 0.25;
//Markers have value between 0 and 1
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

    // Add the marker
    markerIndex = Helper::read_stl("data/models/marker.stl", markervertices, markernormals);

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

//    // Clear unused memory
//    delete [] markervertices;
//    delete [] markernormals;

    marker1 = glm::vec3(0.7,0,1);
    marker2 = glm::vec3(0.2,0,1);
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

glm::vec3 Marker::getMarker1Pos(){
    return marker1;
}
glm::vec3 Marker::getMarker2Pos(){
    return marker2;
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

void Marker::setPositionMarker1(glm::vec3 pos){
    marker1 = pos;
}

void Marker::setPositionMarker2(glm::vec3 pos) {
    marker2 = pos;
}

void Marker::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model){
    glEnable(GL_DEPTH_TEST);

    //Draw Marker 1
    model_marker1      = glm::mat4(1.0f);

    scale(model_marker1, MARKER_SIZE);
    rotate(model_marker1, model);
    translate(model_marker1, model, marker1*10 - glm::vec3(5,5,5));

    drawMarker(projection, view, model_marker1);

    //Draw Marker 2
    model_marker2      = glm::mat4(1.0f);

    scale(model_marker2, MARKER_SIZE);
    rotate(model_marker2, model);
    translate(model_marker2, model, marker2*10 - glm::vec3(5,5,5));

    drawMarker(projection, view, model_marker2);

    glDisable(GL_DEPTH_TEST);
}

// Calculate distance between markers in cm
float Marker::getDistance(float freq, float vel, int depth){
    return length(marker2 - marker1) * (1/freq)*vel*depth/2.0f / 10000.0;
}

bool Marker::checkMouseOnMarker(int imageWidth, int imageHeight, double fov, glm::mat4 cameraToWorld, float x, float y){
    //Generate Ray
    float imageAspectRatio = (imageWidth+0.0f) / (imageHeight+0.0f); // assuming width > height
    float Px = (2 * ((x + 0.5) / imageWidth) - 1) * tan(fov / 2 * M_PI / 180) * imageAspectRatio;
    float Py = (1 - 2 * ((y + 0.5) / imageHeight)) * tan(fov / 2 * M_PI / 180);
    glm::vec4 rayOrigin = glm::vec4(0,0,0,1);
    glm::vec4 rayOriginWorld, rayPWorld;
    rayOriginWorld = cameraToWorld * rayOrigin;
    rayPWorld = cameraToWorld * glm::vec4(Px, Py, -1, 1);
    glm::vec3 rayDirection = rayPWorld - rayOriginWorld;
    glm::normalize(rayDirection);

    //returns -1 for no intersection, 1 for marker1, 2 for marker2
    int intersectMarker = intersect(rayOriginWorld, rayDirection);
    if(intersectMarker != -1){
        std::cout<<"INTERSECTION! "<<intersectMarker<<std::endl;
        return true;
    }

    return false;
}

int Marker::intersect(glm::vec3 rayOrigin, glm::vec3 rayDirection){
    int intersected = -1;

    //loop over each triangle
    for(int x = 0; x < markerIndex/3; x++){
        glm::vec4 v0 = glm::vec4(markervertices[9*x  ], markervertices[9*x+1], markervertices[9*x+2], 1);
        glm::vec4 v1 = glm::vec4(markervertices[9*x+3], markervertices[9*x+4], markervertices[9*x+5], 1);
        glm::vec4 v2 = glm::vec4(markervertices[9*x+6], markervertices[9*x+7], markervertices[9*x+8], 1);

        float t;
        if(rayTriangleIntersect(rayOrigin, rayDirection, model_marker1*v0, model_marker1*v1, model_marker1*v2, t))
            intersected = 1;
        if(rayTriangleIntersect(rayOrigin, rayDirection, model_marker2*v0, model_marker2*v1, model_marker2*v2, t))
            intersected = 2;

        if(intersected != -1)
            break;
    }
    return intersected;
}

bool rayTriangleIntersect(
        const glm::vec3 &orig, const glm::vec3 &dir,
        const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
        float &t)
{
    // compute plane's normal
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v0v2 = v2 - v0;
    // no need to normalize
    glm::vec3 N = glm::cross(v0v1, v0v2);
//    glm::vec3 N = v0v1.crossProduct(v0v2); // N
    float area2 = N.length();

    // Step 1: finding P

    // check if ray and plane are parallel ?
    float NdotRayDirection = glm::dot(N, dir);
//    float NdotRayDirection = N.dotProduct(dir);
    if (fabs(NdotRayDirection) < 0.1) // almost 0
        return false; // they are parallel so they don't intersect !

    // compute d parameter using equation 2
    float d = glm::dot(N, v0);
//    float d = N.dotProduct(v0);

    // compute t (equation 3)
    t = (glm::dot(N, orig) + d) / NdotRayDirection;
//    t = (N.dotProduct(orig) + d) / NdotRayDirection;
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind

    // compute the intersection point using equation 1
    glm::vec3 P = orig + t * dir;

    // Step 2: inside-outside test
    glm::vec3 C; // vector perpendicular to triangle's plane

    // edge 0
    glm::vec3 edge0 = v1 - v0;
    glm::vec3 vp0 = P - v0;
    C = glm::cross(edge0, vp0);
//    C = edge0.crossProduct(vp0);
    if (glm::dot(N, C) < 0) return false; // P is on the right side

    // edge 1
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 vp1 = P - v1;
    C = glm::cross(edge1, vp1);
//    C = edge1.crossProduct(vp1);
    if (glm::dot(N, C) < 0)  return false; // P is on the right side

    // edge 2
    glm::vec3 edge2 = v0 - v2;
    glm::vec3 vp2 = P - v2;
    C = cross(edge2, vp2);
//    C = edge2.crossProduct(vp2);
    if (glm::dot(N, C) < 0) return false; // P is on the right side;

    return true; // this ray hits the triangle
}
