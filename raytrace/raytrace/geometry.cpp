/**
 * @file geometry.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "geometry.hpp"

// borrowed from https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
float Sphere::intersect(const Ray& ray) {
    glm::vec3 sphereToOrigin = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0 * glm::dot(sphereToOrigin, ray.direction);
    float c = glm::dot(sphereToOrigin, sphereToOrigin) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
       return -1.0;
    }
    return (-b - sqrt(discriminant)) / (2.0 * a);
}

float AxisAlignedPlane::intersect(const Ray& ray) {
    float t = (constAxis - ray.direction[constAxisIndex]) / ray.direction[constAxisIndex]; // how long along the trajectory until intersecting plane
    if (t < 0) {
        return -1.0;
    }
    
    glm::vec3 intersection = ray.origin + t * ray.direction; // follow ray out to intersect
    if (!(varAxis11 <= intersection[varAxis1Index] && intersection[varAxis1Index] <= varAxis12 &&
          varAxis21 <= intersection[varAxis2Index] && intersection[varAxis2Index] <= varAxis22)) {
        return -1.0;
    }
    
    return t;
}

glm::vec3 Sphere::normal(const glm::vec3& intersectionPoint) {
    return glm::normalize(intersectionPoint - center);
}

glm::vec3 AxisAlignedPlane::normal(const glm::vec3& intersectionPoint) {
    glm::vec3 normalVector(0, 0, 0);
    normalVector[constAxisIndex] = 1;
    return normalVector;
}
