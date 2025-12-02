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

bool checkRayAABBCollision(const Ray& ray, const AABB& box) {
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 tMin = (box.min - ray.origin) * invDir;
    glm::vec3 tMax = (box.max - ray.origin) * invDir;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

    return tNear <= tFar && tFar >= 0.0f;
}
