#include "../include/collisions.h"
#include <glm/glm.hpp>

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane) {
    float signedDistance = glm::dot(plane.normal, sphere.center) + plane.distance;
    return glm::abs(signedDistance) <= sphere.radius;
}
