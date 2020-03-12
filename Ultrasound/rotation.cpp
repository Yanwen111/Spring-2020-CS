#include <glm/gtc/quaternion.hpp>
#include "rotation.h"

glm::mat4 Rotation::convertRotationMatrix(float w, float x, float y, float z){
    //glm::quat myQuat = glm::quat(w,x,y,z);
    //glm::mat4 rotationMatrix = glm::mat4_cast(myQuat);

    glm::vec3 eulers = convertToEulerAngle(w, x, y, z);
    glm::mat4 rotationMatrix = glm::mat4(1.0f);

    rotationMatrix = glm::rotate(rotationMatrix, eulers.x , glm::vec3(1, 0, 0));
    rotationMatrix = glm::rotate(rotationMatrix, float(-1 * eulers.z), glm::vec3(0, 1, 0));
    rotationMatrix = glm::rotate(rotationMatrix, eulers.y, glm::vec3(0, 0, 1));


    return rotationMatrix;
}

glm::vec3 Rotation::convertToEulerAngle(float w, float x, float y, float z){
    float roll, pitch, yaw;
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

glm::vec4 Rotation::convertToQuaterion(float yaw, float pitch, float roll) {
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