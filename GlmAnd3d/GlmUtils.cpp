#include "GlmUtils.h"

bool vectorsParallel(const glm::vec3& v1, const glm::vec3& v2, float epsilon)
{
    // Compute the cross product
    glm::vec3 crossProd = glm::cross(v1, v2);

    // Check if the squared length of the cross product is near zero
    return glm::length2(crossProd) < epsilon;
}

std::string toString(glm::vec3 vec)
{
    return std::format("[{:.2f}, {:.2f}, {:.2f}]", vec.x, vec.y, vec.z);
}