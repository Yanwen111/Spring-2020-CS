#include <glm/glm.hpp>

namespace Rotation{

    glm::mat4 convertRotationMatrix(float w, float x, float y, float z);

    //Haven't tested these methods yet
    glm::vec3 convertToEulerAngle(float w, float x, float y, float z);
    glm::vec4 convertToQuaterion(float yaw, float pitch, float roll);
}
