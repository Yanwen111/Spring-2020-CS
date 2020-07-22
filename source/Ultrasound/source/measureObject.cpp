#include "measureObject.h"
//DensityMap* gridPointer
MeasureObject::MeasureObject(){};
MeasureObject::MeasureObject(DensityMap* gridPointer){
//    pos = glm::vec3(
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//            static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    pos = glm::vec3(0,0,0);

    myColor = glm::vec3(.831, .882, .949);
    mySize = 1.0;

    cubeSetUp();
    sphereSetUp();
    cylinderSetUp();

    myGrid = gridPointer;
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

void MeasureObject::cylinderSetUp() {
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
            "   FragColor = vec4(result, 0.7);							 \n"
            "}										                     \n";

    cylinderShader = Shader(vmarker.c_str(), fmarker.c_str(), false);

    // Add the cylinder (radius = 1, depth = 2, circles facing y-axis direction)
    cylinderIndex = Helper::read_stl("config_file/models/cylinder-r-1-d-2.stl", cylindervertices, cylindernormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &cylinderVBO);
    glGenBuffers(1, &cylinderNormalsVBO);
    glGenVertexArrays(1, &cylinderVAO);

    glBindVertexArray(cylinderVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, cylinderIndex * sizeof(GLfloat), cylindervertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, cylinderNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, cylinderIndex * sizeof(GLfloat), cylindernormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
}

void MeasureObject::sphereSetUp() {
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
            "   FragColor = vec4(result, 0.7);							 \n"
            "}										                     \n";

    sphereShader = Shader(vmarker.c_str(), fmarker.c_str(), false);

    // Add the marker
    sphereIndex = Helper::read_stl("config_file/models/sphere.stl", spherevertices, spherenormals);

    //Set up OpenGL buffers
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereNormalsVBO);
    glGenVertexArrays(1, &sphereVAO);

    glBindVertexArray(sphereVAO);

    //position attribute
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereIndex * sizeof(GLfloat), spherevertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    //normals attribute
    glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereIndex * sizeof(GLfloat), spherenormals, GL_STATIC_DRAW);

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

//returns radius in mm
float MeasureObject::getRadius(float freq, float vel, int depth) {
    if(displayObject == 0)
        return mySize * (1/freq)*vel*depth/2.0f / 10000.0f;
    else if(displayObject == 1)
        return myCylinderRadius * (1/freq)*vel*depth/2.0f / 10000.0f;
    else return -1;

}

glm::vec3 MeasureObject::getDirection() {
    return upVector;
}

//calculate the best fit sphere
void MeasureObject::calculateSphere() {

    displayObject = 0;

    glm::vec3 center;
    float radius;

    for(int x = 0; x < 50; x++) {
        findThreshold();
        for(int c = 0; c < 10; c++) {
            center = getCenter();
            pos = center;
        }
        radius = getRadius(center);
        if(radius <= 0) {
            mySize = mySize / 1.15;
            break;
        }
        mySize = radius * 1.15;
    }

    pos = center;
    if(radius > 0)
        mySize = radius;

    showCube = false;
}

//calculate the best fit cylinder
void MeasureObject::calculateCylinder() {

    displayObject = 1;
//
    glm::vec3 center;
    float radius;
//
//    for(int x = 0; x < 50; x++) {
        findThreshold();
        for(int c = 0; c < 10; c++) {
            center = getCenter();
            pos = center;
        }

        upVector = getDirection(center);
//        std::cout<<"x: "<<upVector.x<<" Y: "<<upVector.y<<" Z: "<<upVector.z<<std::endl;
        pos = center;

        radius = getRadiusCylinder(center, upVector);

        myCylinderRadius = radius;

//        radius = getRadius(center);
//        if(radius <= 0) {
//            mySize = mySize / 1.15;
//            break;
//        }
//        mySize = radius * 1.15;
//    }
//
//    pos = center;
//    if(radius > 0)
//        mySize = radius;

    showCube = false;
}

//calculates threshold using the average value...
void MeasureObject::findThreshold() {
    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());

//    float valSum = 0;
//    int numPoints = 0;
    std::vector<float> points;

    for(int x = 0; x < cubeDim; x++) {
        for(int y = 0; y < cubeDim; y++) {
            for(int z = 0; z < cubeDim; z++) {
                //position in [-5,5] coordinates
                glm::vec3 pointPos = glm::vec3(
                        mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
                        mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
                        mySize*2/float(cubeDim) * float(z) + pos.z - mySize
                );

                //position in [0, 1] coordinates
                glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;

                float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);

                points.push_back(val);
            }
        }
    }

    std::sort(points.begin(), points.end());
    myThreshold = points[points.size()/2];

}


float MeasureObject::getThreshold() {
    return myThreshold;
}

//http://www.jpe-innovations.com/downloads/Fit-sphere-through-points.pdf to calculate radius
float MeasureObject::getRadius(glm::vec3 center) {

    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());

    float valSum = 0;
    float sum = 0;

    for(int x = 0; x < cubeDim; x++) {
        for(int y = 0; y < cubeDim; y++) {
            for(int z = 0; z < cubeDim; z++) {
                //position in [-5,5] coordinates
                glm::vec3 pointPos = glm::vec3(
                        mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
                        mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
                        mySize*2/float(cubeDim) * float(z) + pos.z - mySize
                );

                //position in [0, 1] coordinates
                glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;

                float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);

                if(val > myThreshold) {
                    valSum += val;
                    sum += val*((pointPos.x-center.x)*(pointPos.x-center.x) +
                            (pointPos.y-center.y)*(pointPos.y-center.y) +
                            (pointPos.z-center.z)*(pointPos.z-center.z));
                }
            }
        }
    }
//    std::cout<<"RADIUS: "<<sqrt(sum / valSum)<<std::endl;
    if(valSum > 0) sum = sum / valSum;
    return sqrt(sum);
}

float MeasureObject::getRadiusCylinder(glm::vec3 center, glm::vec3 direction) {
    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());

    float sum = 0;
    float valSum = 0;

    for(int x = 0; x < cubeDim; x++) {
        for(int y = 0; y < cubeDim; y++) {
            for(int z = 0; z < cubeDim; z++) {
                //position in [-5,5] coordinates
                glm::vec3 pointPos = glm::vec3(
                        mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
                        mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
                        mySize*2/float(cubeDim) * float(z) + pos.z - mySize
                );

                //position in [0, 1] coordinates
                glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;

                float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);


                if(val > myThreshold) {
                    glm::vec3 centeredPoint = pointPos - center;
                    float dist = glm::dot(centeredPoint, direction);
                    glm::vec3 pointOnLine = center + dist * direction;

                    sum += val*((pointPos.x-pointOnLine.x)*(pointPos.x-pointOnLine.x) +
                                (pointPos.y-pointOnLine.y)*(pointPos.y-pointOnLine.y) +
                                (pointPos.z-pointOnLine.z)*(pointPos.z-pointOnLine.z));

                    valSum += val;

                }
            }
        }
    }

    if(valSum > 0)
        sum = sum / valSum;
    return sqrt(sum);
}

//weighted average of points in the box above the threshold
glm::vec3 MeasureObject::getCenter() {

    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());

    glm::vec3 center = glm::vec3(0,0,0);
    float valSum = 0;

    for(int x = 0; x < cubeDim; x++) {
        for(int y = 0; y < cubeDim; y++) {
            for(int z = 0; z < cubeDim; z++) {
                //position in [-5,5] coordinates
                glm::vec3 pointPos = glm::vec3(
                        mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
                        mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
                        mySize*2/float(cubeDim) * float(z) + pos.z - mySize
                );

                //position in [0, 1] coordinates
                glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;

                float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);


                if(val > myThreshold) {
                    center += val * pointPos;
                    valSum += val;
                }
            }
        }
    }

    if(valSum > 0)
        center = center / valSum;
    return center;
}

//weighted line fit to points. https://zalo.github.io/blog/line-fitting/
glm::vec3 MeasureObject::getDirection(glm::vec3 center) {
    glm::vec3 direction = glm::vec3(0,1,0);
    int num_iters = 10;
    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());

    for(int i = 0; i < num_iters; i++) {

        glm::vec3 nextDirection = glm::vec3(0,0,0);
        float valSum = 0;

        for(int x = 0; x < cubeDim; x++) {
            for(int y = 0; y < cubeDim; y++) {
                for(int z = 0; z < cubeDim; z++) {
                    //position in [-5,5] coordinates
                    glm::vec3 pointPos = glm::vec3(
                            mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
                            mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
                            mySize*2/float(cubeDim) * float(z) + pos.z - mySize
                    );

                    //position in [0, 1] coordinates
                    glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;

                    float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);


                    if(val > myThreshold) {
                        glm::vec3 centeredPoint = pointPos - center;
                        nextDirection += val*glm::dot(centeredPoint, direction) * centeredPoint;
                        valSum += val;
                    }
                }
            }
        }

        nextDirection = nextDirection / valSum;
        direction = glm::normalize(nextDirection);
    }
    return direction;
}

//enter the selecting area with cube option
void MeasureObject::selectArea() {
    showCube = true;
}

float* MeasureObject::getSize(){
    return &mySize;
}
void MeasureObject::setSize(float size){
    mySize = size;
}


//returns the position in world coordinates
glm::vec3 MeasureObject::getPosWorld(float freq, float vel, int depth) {
//    return mySize * (1/freq)*vel*depth/2.0f / 10000.0f;
    glm::vec3 worldPos = pos;
    worldPos.x = worldPos.x * (1/freq)*vel*depth/2.0f / 100000.0f;
    worldPos.y = (5 - worldPos.y)* (1/freq)*vel*depth/2.0f / 100000.0f;
    worldPos.z = worldPos.z * (1/freq)*vel*depth/2.0f / 100000.0f;
    return worldPos;
}

glm::vec3 MeasureObject::getPos() {
    return pos;
}

void MeasureObject::setPos(glm::vec3 inPos) {
    pos = inPos;
}

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

void MeasureObject::drawSphere(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker){
    // Drawing the marker
    sphereShader.use();
    sphereShader.setMat4("projection", projection);
    sphereShader.setMat4("view", view);
    sphereShader.setMat4("model", model_marker);

    sphereShader.setVec3("lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
    sphereShader.setVec3("objectColor", myColor);

    sphereShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereIndex/3);
}

void MeasureObject::drawCylinder(glm::mat4 projection, glm::mat4 view, glm::mat4 model_marker){
    // Drawing the marker
    cylinderShader.use();
    cylinderShader.setMat4("projection", projection);
    cylinderShader.setMat4("view", view);
    cylinderShader.setMat4("model", model_marker);

    cylinderShader.setVec3("lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
    cylinderShader.setVec3("objectColor", myColor);

    cylinderShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(cylinderVAO);
    glDrawArrays(GL_TRIANGLES, 0, cylinderIndex/3);
}

void MeasureObject::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model_in) {
    glEnable(GL_DEPTH_TEST);

    if(showCube) {
        model = glm::mat4(1.0f);
        scaleCube(model, mySize);
        rotateCube(model, model_in);
        translateCube(model, model_in, pos);
        drawCube(projection, view, model);
    }
    else if(displayObject == 0) {
        model = glm::mat4(1.0f);
        scaleCube(model, mySize);
        rotateCube(model, model_in);
        translateCube(model, model_in, pos);
        drawSphere(projection, view, model);
    }
    else if(displayObject == 1) {
        model = glm::mat4(1.0f);

        //scale radius
        glm::vec3 scale(myCylinderRadius, mySize, myCylinderRadius);
        model = glm::scale(glm::mat4(1.0f), scale) * model;

        //rotate cylinder to match new up direction
        glm::vec3 origUp = glm::vec3(0,1,0);
        float angle = acos( glm::dot(origUp, upVector));
        glm::vec3 axis = glm::cross(origUp, upVector);
        float c = cos(angle); //1
        float s = sin(angle); //0
        float t = 1 - c; //0
        auto rotMatrix = glm::mat4(1);
        //rotation matrix from axis and angle  https://en.wikipedia.org/wiki/Rotation_matrix
        rotMatrix[0] = glm::vec4(c + axis.x*axis.x*t, axis.y*axis.x*t + axis.z*s, axis.z*axis.x*t - axis.y*s, 0);
        rotMatrix[1] = glm::vec4(axis.x*axis.y*t - axis.z*s, c + axis.y*axis.y*t, axis.z*axis.y*t + axis.x*s, 0);
        rotMatrix[2] = glm::vec4(axis.x*axis.z*t + axis.y*s, axis.y*axis.z*t - axis.x*s, c + axis.z*axis.z*t, 0);
        model = rotMatrix * model;

        rotateCube(model, model_in);
        translateCube(model, model_in, pos);
        drawCylinder(projection, view, model);
    }

    glDisable(GL_DEPTH_TEST);
}

void MeasureObject::setIntersected(bool value){
    isIntersected = value;
}

bool MeasureObject::getIntersected(){
    return isIntersected;
}

int MeasureObject::getDisplayObject() {
    return displayObject;
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

    if(!showCube) return intersected;

    //check if ray intersects the cube

    //loop over each triangle
    for (int x = 0; x < cubeIndex / 9; x++) {
        glm::vec4 v0 = glm::vec4(cubevertices[9 * x], cubevertices[9 * x + 1], cubevertices[9 * x + 2], 1);
        glm::vec4 v1 = glm::vec4(cubevertices[9 * x + 3], cubevertices[9 * x + 4], cubevertices[9 * x + 5],
                                 1);
        glm::vec4 v2 = glm::vec4(cubevertices[9 * x + 6], cubevertices[9 * x + 7], cubevertices[9 * x + 8],
                                 1);

        if (rayTriangleIntersect(rayOrigin, rayDirection, model * v0, model * v1, model * v2,
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
