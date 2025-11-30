#include "../include/collisions.h"
#include <glm/glm.hpp>
#include "../include/matrices.h"

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane) {
    float signedDistance = dotproduct(glm::vec4(plane.normal, 0.0f), glm::vec4(sphere.center, 0.0f)) + plane.distance;
    return glm::abs(signedDistance) <= sphere.radius;
}

float checkRayPlaneCollision(const Ray& ray, const Plane& plane) {
    float denom = dotproduct(glm::vec4(plane.normal, 0.0f), glm::vec4(ray.direction, 0.0f));
    if (glm::abs(denom) > 1e-6) { // Evita divisão por zero
        float t = -(dotproduct(glm::vec4(plane.normal, 0.0f), glm::vec4(ray.origin, 0.0f)) + plane.distance) / denom;
        return t;
    }
    return -1.0f; // Sem colisão
}