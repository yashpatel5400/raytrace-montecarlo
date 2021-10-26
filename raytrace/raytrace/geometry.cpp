/**
 * @file geometry.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "geometry.hpp"

#include <iostream>

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
    normalVector[constAxisIndex] = facingAxis ? 1 : -1;
    return normalVector;
}

Box::Box(const glm::vec3& minCorner,
         const glm::vec3& maxCorner,
         std::shared_ptr<Material> material) : Geometry(material) {
    sides.push_back(std::make_shared<XYPlane>(minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, minCorner.z, false, material)); // back
    sides.push_back(std::make_shared<XYPlane>(minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, maxCorner.z, true, material)); // front
    sides.push_back(std::make_shared<XZPlane>(minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, minCorner.y, false, material)); // bottom
    sides.push_back(std::make_shared<XZPlane>(minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, maxCorner.y, true, material)); // top
    sides.push_back(std::make_shared<YZPlane>(minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, minCorner.x, false, material)); // left
    sides.push_back(std::make_shared<YZPlane>(minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, maxCorner.x, true, material)); // right
}

float Box::intersect(const Ray& ray) {
    std::shared_ptr<AxisAlignedPlane> closestSide;
    float closestIntersection = std::numeric_limits<float>::max();
    
    for (const std::shared_ptr<AxisAlignedPlane>& side : sides) {
        float t = side->intersect(ray);
        if (t > 0.0 && t < closestIntersection) {
            closestIntersection = t;
            closestSide = side;
        }
    }
    
    if (closestIntersection < std::numeric_limits<float>::max()) {
        glm::vec3 intersection = ray.origin + closestIntersection * ray.direction;
        intersectedSideNormal = closestSide->normal(intersection);
        return closestIntersection;
    }
    
    intersectedSideNormal = glm::vec3(0, 0, 0); // no normal if no intersection
    return -1.0;
}

glm::vec3 Box::normal(const glm::vec3& intersectionPoint) {
    // this is poor design, but think fixing it would be a bit of a pain, so just storing the value from intersect
    // and just assuming that the normal is always gonna be called after a call to intersect()
    return intersectedSideNormal;
}
