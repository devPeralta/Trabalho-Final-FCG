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

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane);
bool checkSphereSphereCollision(const Sphere& s1, const Sphere& s2);
bool checkRayAABBCollision(const Ray& ray, const AABB& box);

#endif //TRABALHO_FINAL_FCG_COLLISIONS_H