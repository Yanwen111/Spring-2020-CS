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

float MeasureObject::getRadius(float freq, float vel, int depth) {
    return mySize / 10.0f * (1/freq)*vel*depth/2.0f / 10000.0f;
}

//calculate the best fit sphere
void MeasureObject::calculate() {

//    struct PointCalc P = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//    prepareData(P);
//    std::cout<<"Prepared Data: "<<P.npoints<<" " <<P.Xsum<<" "<<P.Ysum<<" "<<P.Zsum<<std::endl;
//
//    float A, B, C, Rsq;
//    std::cout<<"Calculating Least Squares!"<<std::endl;
//    calculateLeastSquaresSphere(P, A, B, C, Rsq);
//    std::cout<<"Rad: "<<sqrt(Rsq)<<" X "<<A<<" Y "<<B<<" Z "<<C<<std::endl;
//
//    pos = glm::vec3(A, B, C);
//    mySize = sqrt(Rsq);


    findThreshold();
    glm::vec3 center = getCenter();
    float radius = getRadius(center);

    pos = center;
    mySize = radius;

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

//    myThreshold = valSum / float(numPoints);
    std::cout<<"Threshold: "<<myThreshold<<std::endl;
}

//float MeasureObject::getRsqVal() {
//    return rsqVal;
//}

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
    std::cout<<"RADIUS: "<<sqrt(sum / valSum)<<std::endl;
    return sqrt(sum / valSum);
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

    center = center / valSum;
    std::cout<<"CENTER: "<<center.x<<" "<<center.y<<" "<<center.z<<std::endl;
    return center;
}

//void MeasureObject::prepareData(PointCalc& P) {
//
//    int cubeDim = (int)(mySize / 5.0 * myGrid->getDim());
//
//    glm::vec3 center = glm::vec3(0,0,0);
//    float valSum = 0;
//
//    for(int x = 0; x < cubeDim; x++) {
//        for(int y = 0; y < cubeDim; y++) {
//            for(int z = 0; z < cubeDim; z++) {
//                //position in [-5,5] coordinates
//                glm::vec3 pointPos = glm::vec3(
//                        mySize*2/float(cubeDim) * float(x) + pos.x - mySize,
//                        mySize*2/float(cubeDim) * float(y) + pos.y - mySize,
//                        mySize*2/float(cubeDim) * float(z) + pos.z - mySize
//                );
//
//                //position in [0, 1] coordinates
//                glm::vec3 gridPos = (pointPos + glm::vec3(5,5,5))/10.0;
//
//                float val = myGrid->readCellInterpolated(gridPos.x, gridPos.y, gridPos.z);
//
//
//                if(val > myThreshold) {
//                    center += val * pointPos;
//                    valSum += val;
//
////                    P.npoints += 1;
////
////                    P.Xsum += pointPos.x;
////                    P.Xsumsq += pointPos.x * pointPos.x;
////                    P.Xsumcube += pointPos.x * pointPos.x * pointPos.x;
////
////                    P.Ysum += pointPos.y;
////                    P.Ysumsq += pointPos.y * pointPos.y;
////                    P.Ysumcube += pointPos.y * pointPos.y * pointPos.y;
////
////                    P.Zsum += pointPos.z;
////                    P.Zsumsq += pointPos.z * pointPos.z;
////                    P.Zsumcube += pointPos.z * pointPos.z * pointPos.z;
////
////                    P.XYsum += pointPos.x + pointPos.y;
////                    P.XZsum += pointPos.x + pointPos.z;
////                    P.YZsum += pointPos.y + pointPos.z;
////
////                    P.X2Ysum += pointPos.x * pointPos.x + pointPos.y;
////                    P.X2Zsum += pointPos.x * pointPos.x + pointPos.z;
////                    P.Y2Xsum += pointPos.y * pointPos.y + pointPos.x;
////                    P.Y2Zsum += pointPos.y * pointPos.y + pointPos.z;
////                    P.Z2Xsum += pointPos.z * pointPos.z + pointPos.x;
////                    P.Z2Ysum += pointPos.z * pointPos.z + pointPos.y;
//                }
//            }
//        }
//    }
//
//    center = center / valSum;
//    std::cout<<"CENTER: "<<center.x<<" "<<center.y<<" "<<center.z<<std::endl;
//
//    pos = center;
//
//}
//
//void MeasureObject::calculateLeastSquaresSphere(PointCalc P, float& A, float& B, float& C, float& Rsq) {
////Least Squares Fit a sphere A,B,C with radius squared Rsq to 3D data
////
////    P is a structure that has been computed with the data earlier.
////    P.npoints is the number of elements; the length of X,Y,Z are identical.
////    P's members are logically named.
////
////    X[n] is the x component of point n
////    Y[n] is the y component of point n
////    Z[n] is the z component of point n
////
////    A is the x coordiante of the sphere
////    B is the y coordiante of the sphere
////    C is the z coordiante of the sphere
////    Rsq is the radius squared of the sphere.
////
////This method should converge; maybe 5-100 iterations or more.
//
//    double Xn = P.Xsum/P.npoints;        //sum( X[n] )
//    std::cout<<"Xn"<<Xn<<std::endl;
//    double Xn2 = P.Xsumsq/P.npoints;    //sum( X[n]^2 )
//    std::cout<<"Xn"<<Xn2<<std::endl;
//    double Xn3 = P.Xsumcube/P.npoints;    //sum( X[n]^3 )
//    std::cout<<"Xn"<<Xn3<<std::endl;
//    double Yn = P.Ysum/P.npoints;        //sum( Y[n] )
//    double Yn2 = P.Ysumsq/P.npoints;    //sum( Y[n]^2 )
//    double Yn3 = P.Ysumcube/P.npoints;    //sum( Y[n]^3 )
//    double Zn = P.Zsum/P.npoints;        //sum( Z[n] )
//    double Zn2 = P.Zsumsq/P.npoints;    //sum( Z[n]^2 )
//    double Zn3 = P.Zsumcube/P.npoints;    //sum( Z[n]^3 )
//
//    double XY = P.XYsum/P.npoints;        //sum( X[n] * Y[n] )
//    double XZ = P.XZsum/P.npoints;        //sum( X[n] * Z[n] )
//    double YZ = P.YZsum/P.npoints;        //sum( Y[n] * Z[n] )
//    double X2Y = P.X2Ysum/P.npoints;    //sum( X[n]^2 * Y[n] )
//    double X2Z = P.X2Zsum/P.npoints;    //sum( X[n]^2 * Z[n] )
//    double Y2X = P.Y2Xsum/P.npoints;    //sum( Y[n]^2 * X[n] )
//    double Y2Z = P.Y2Zsum/P.npoints;    //sum( Y[n]^2 * Z[n] )
//    double Z2X = P.Z2Xsum/P.npoints;    //sum( Z[n]^2 * X[n] )
//    double Z2Y = P.Z2Ysum/P.npoints;    //sum( Z[n]^2 * Y[n] )
//
////Reduction of multiplications
//    double F0 = Xn2 + Yn2 + Zn2;
//    double F1 = 0.5*F0;
//    double F2 = -8.0*(Xn3 + Y2X + Z2X);
//    double F3 = -8.0*(X2Y + Yn3 + Z2Y);
//    double F4 = -8.0*(X2Z + Y2Z + Zn3);
//
////Set initial conditions:
//    A = Xn;
//    B = Yn;
//    C = Zn;
//
////First iteration computation:
//    double A2 = A*A;
//    double B2 = B*B;
//    double C2 = C*C;
//    double QS = A2 + B2 + C2;
//    double QB = - 2*(A*Xn + B*Yn + C*Zn);
//
////Set initial conditions:
//    Rsq = F0 + QB + QS;
//
////First iteration computation:
//    double Q0 = 0.5*(QS - Rsq);
//    double Q1 = F1 + Q0;
//    double Q2 = 8*( QS - Rsq + QB + F0 );
//    double aA,aB,aC,nA,nB,nC,dA,dB,dC;
//
////Iterate N times, ignore stop condition.
//    int n = 0;
//    while( n != N ){
//        n++;
//
//        //Compute denominator:
//        aA = Q2 + 16*(A2 - 2*A*Xn + Xn2);
//        aB = Q2 + 16*(B2 - 2*B*Yn + Yn2);
//        aC = Q2 + 16*(C2 - 2*C*Zn + Zn2);
//        aA = (aA == 0) ? 1.0 : aA;
//        aB = (aB == 0) ? 1.0 : aB;
//        aC = (aC == 0) ? 1.0 : aC;
//
//        //Compute next iteration
//        nA = A - ((F2 + 16*( B*XY + C*XZ + Xn*(-A2 - Q0) + A*(Xn2 + Q1 - C*Zn - B*Yn) ) )/aA);
//        nB = B - ((F3 + 16*( A*XY + C*YZ + Yn*(-B2 - Q0) + B*(Yn2 + Q1 - A*Xn - C*Zn) ) )/aB);
//        nC = C - ((F4 + 16*( A*XZ + B*YZ + Zn*(-C2 - Q0) + C*(Zn2 + Q1 - A*Xn - B*Yn) ) )/aC);
//
//        //Check for stop condition
//        dA = (nA - A);
//        dB = (nB - B);
//        dC = (nC - C);
//        if( (dA*dA + dB*dB + dC*dC) <= Nstop ){ std::cout<<"BREAKING!"<<std::endl; break; }
//
//        //Compute next iteration's values
//        A = nA;
//        B = nB;
//        C = nC;
//        A2 = A*A;
//        B2 = B*B;
//        C2 = C*C;
//        QS = A2 + B2 + C2;
//        QB = - 2*(A*Xn + B*Yn + C*Zn);
//        Rsq = F0 + QB + QS;
//        Q0 = 0.5*(QS - Rsq);
//        Q1 = F1 + Q0;
//        Q2 = 8*( QS - Rsq + QB + F0 );
//    }
//}

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

    // Set lights
//    if(isIntersected){
//        sphereShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
//        sphereShader.setVec3("objectColor", myColor*1.5f);
//    }
//    else {
        sphereShader.setVec3("lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
        sphereShader.setVec3("objectColor", myColor);
//    }
    sphereShader.setVec3("lightPos", glm::vec3(0.0f, 15.0f, 5.0f));

    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereIndex/3);
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
    else {
        model = glm::mat4(1.0f);
        scaleCube(model, mySize);
        rotateCube(model, model_in);
        translateCube(model, model_in, pos);
        drawSphere(projection, view, model);
    }

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
