#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <glm/glm.hpp>

struct Sphere {
    glm::vec3 center;
    float radius;
};

struct Plane {
    glm::vec3 normal;
    float distance;
};

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane);

#endif // COLLISIONS_H
