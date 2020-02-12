#include <glm.hpp>

#ifndef ETHAN_OPENGL_ROTATIONS_H
#define ETHAN_OPENGL_ROTATIONS_H

#endif //ETHAN_OPENGL_ROTATIONS_H

class Rotation {
public:
    explicit Rotation(glm::vec4 euler);
    explicit Rotation(glm::vec3 quat);
    glm::vec3 getEulerAngle();
    glm::vec4 getQuaternion();
    glm::vec3 convertToEulerAngle();
    glm::vec4 convertToQuaterion();
    glm::mat4 setRotationMatrix();
    glm::mat4 getRotationMatrix();

    float roll, pitch, yaw;
    float w, x, y, z;
    glm::mat4 rotationMatrix;

};
