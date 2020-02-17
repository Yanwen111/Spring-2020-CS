#define _USE_MATH_DEFINES
#include <cmath>
#include "rotations.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

Rotation::Rotation(glm::vec4 quat){
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;

    glm::vec3 euler = convertToEulerAngle();
    roll = euler.x;
    pitch = euler.y;
    yaw = euler.z;

    rotationMatrix = setRotationMatrix();
}

Rotation::Rotation(glm::vec3 euler){
    roll = euler.x;
    pitch = euler.y;
    yaw = euler.z;

    glm::vec4 quat = convertToQuaterion();
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;

    rotationMatrix = setRotationMatrix();
}

glm::vec3 Rotation::getEulerAngle(){
    return glm::vec3(roll, pitch, yaw);
}

glm::vec4 Rotation::getQuaternion(){
    return glm::vec4(x, y, z, w);
}

glm::mat4 Rotation::getRotationMatrix(){
    return rotationMatrix;
}

glm::mat4 Rotation::setRotationMatrix(){
    rotationMatrix = glm::mat4(1.0f);

    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(roll), glm::vec3(1, 0, 0));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(pitch), glm::vec3(0, 1, 0));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(yaw), glm::vec3(0, 0, 1));

    return rotationMatrix;
}

glm::vec3 Rotation::convertToEulerAngle(){
    // roll (x-axis rotation)
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    roll = atan2f(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    float sinp = 2 * (w * y - z * x);
    if (std::abs(sinp) >= 1)
        pitch = std::copysignf(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asinf(sinp);

    // yaw (z-axis rotation)
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    yaw = atan2f(siny_cosp, cosy_cosp);

    return glm::vec3(roll, pitch, yaw);
}

glm::vec4 Rotation::convertToQuaterion() {
    // Abbreviations for the various angular functions
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    glm::vec4 quat = glm::vec4();
    quat.w = cy * cp * cr + sy * sp * sr;
    quat.x = cy * cp * sr - sy * sp * cr;
    quat.y = sy * cp * sr + cy * sp * cr;
    quat.z = sy * cp * cr - cy * sp * sr;

    return quat;
}

