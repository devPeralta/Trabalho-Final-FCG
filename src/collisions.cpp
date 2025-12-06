#include "../include/collisions.h"
#include <glm/glm.hpp>
#include "../include/matrices.h"

bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane) {
    float signedDistance = dotproduct(glm::vec4(plane.normal, 0.0f),
                                         glm::vec4(sphere.center, 0.0f)) + plane.distance;
    return glm::abs(signedDistance) <= sphere.radius;
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

bool checkSphereSphereCollision(const Sphere& sphere1, const Sphere& sphere2) {
    float distance = glm::distance(sphere1.center, sphere2.center);
    float sum_radii = sphere1.radius + sphere2.radius;
    return distance <= sum_radii;
}
