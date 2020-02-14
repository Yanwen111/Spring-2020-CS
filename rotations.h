#include <glm.hpp>
#include <fstream>

#ifndef ETHAN_OPENGL_ROTATIONS_H
#define ETHAN_OPENGL_ROTATIONS_H

#endif //ETHAN_OPENGL_ROTATIONS_H

class Rotation {
public:
    //Assumes input as a vec4 with values (x, y, z, w)
//    explicit Rotation(glm::vec4 quat);
//    explicit Rotation(glm::vec3 euler);
//    glm::vec3 getEulerAngle();
//    glm::vec4 getQuaternion();
    glm::vec3 convertToEulerAngle();
//    glm::vec4 convertToQuaterion();
    glm::mat4 convertRotationMatrix();
//    glm::mat4 getRotationMatrix();

    Rotation(const std::string& inputFileName);
    glm::mat4 getOrientation();
    void readNextLine();
    void closeFile();
    void testData();

private:
    float roll, pitch, yaw;
    float w, x, y, z;
    glm::mat4 rotationMatrix;
    std::ifstream file;

    void parseLine(char *str);
};
