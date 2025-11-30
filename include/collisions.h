#ifndef TRABALHO_FINAL_FCG_COLLISIONS_H
#define TRABALHO_FINAL_FCG_COLLISIONS_H

#include <glm/vec3.hpp>

struct Sphere {
    glm::vec3 center;
    float radius;
};

struct Plane {
    glm::vec3 normal;
    float distance;
};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane);
float checkRayPlaneCollision(const Ray& ray, const Plane& plane);

#endif //TRABALHO_FINAL_FCG_COLLISIONS_H

