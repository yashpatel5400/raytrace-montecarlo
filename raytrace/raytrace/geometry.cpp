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
float Sphere::intersect(const Ray& ray, glm::vec3& intersection) {
    glm::vec3 sphereToOrigin = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0 * glm::dot(sphereToOrigin, ray.direction);
    float c = glm::dot(sphereToOrigin, sphereToOrigin) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
       return -1.0;
    }
    float t = (-b - sqrt(discriminant)) / (2.0 * a);
    intersection = ray.origin + t * ray.direction;
    return t;
}

float AxisAlignedPlane::intersect(const Ray& ray, glm::vec3& intersection) {
    return intersect(ray, intersection, glm::vec3(0, 0, 0));
}

float AxisAlignedPlane::intersect(const Ray& ray, glm::vec3& intersection, const glm::vec3& offset) {
    Ray rotatedRay = ray;
    rotatedRay.origin.x = cos(yAxisRotation) * ray.origin.x - sin(yAxisRotation) * ray.origin.z;
    rotatedRay.origin.z = sin(yAxisRotation) * ray.origin.x + cos(yAxisRotation) * ray.origin.z;

    rotatedRay.direction.x = cos(yAxisRotation) * ray.direction.x - sin(yAxisRotation) * ray.direction.z;
    rotatedRay.direction.z = sin(yAxisRotation) * ray.direction.x + cos(yAxisRotation) * ray.direction.z;
    
    rotatedRay.origin -= offset;
    
    // how long along the trajectory until intersecting plane
    float t = (constAxis - rotatedRay.origin[constAxisIndex]) / rotatedRay.direction[constAxisIndex];
    if (t < 0) {
        return -1.0;
    }
    
    glm::vec3 potentialIntersection = rotatedRay.origin + t * rotatedRay.direction; // follow ray out to intersect
    if (!(varAxis11 <= potentialIntersection[varAxis1Index] && potentialIntersection[varAxis1Index] <= varAxis12 &&
          varAxis21 <= potentialIntersection[varAxis2Index] && potentialIntersection[varAxis2Index] <= varAxis22)) {
        return -1.0;
    }
    
    intersection =  potentialIntersection;
    intersection += offset;
    intersection.x =  cos(yAxisRotation) * potentialIntersection.x + sin(yAxisRotation) * potentialIntersection.z;
    intersection.z = -sin(yAxisRotation) * potentialIntersection.x + cos(yAxisRotation) * potentialIntersection.z;
    return t;
}

glm::vec3 Sphere::normal(const glm::vec3& intersectionPoint) {
    return glm::normalize(intersectionPoint - center);
}

glm::vec3 AxisAlignedPlane::normal(const glm::vec3& intersectionPoint) {
    glm::vec3 normalVector(0, 0, 0);
    normalVector[constAxisIndex] = facingAxis ? 1 : -1;
    
    glm::vec3 rotatedNormal = normalVector;
    rotatedNormal.x =  cos(yAxisRotation) * normalVector.x + sin(yAxisRotation) * normalVector.z;
    rotatedNormal.z = -sin(yAxisRotation) * normalVector.x + cos(yAxisRotation) * normalVector.z;
    return rotatedNormal;
}

Box::Box(const glm::vec3& minCorner,
         const glm::vec3& maxCorner,
         const glm::vec3& offset,
         const float yAxisRotation,
         std::shared_ptr<Material> material) : offset(offset), Geometry(material) {
    sides.push_back(std::make_shared<XYPlane>(minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, minCorner.z, false, yAxisRotation, material)); // back
    sides.push_back(std::make_shared<XYPlane>(minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, maxCorner.z, true, yAxisRotation, material)); // front
    sides.push_back(std::make_shared<XZPlane>(minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, minCorner.y, false, yAxisRotation, material)); // bottom
    sides.push_back(std::make_shared<XZPlane>(minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, maxCorner.y, true, yAxisRotation, material)); // top
    sides.push_back(std::make_shared<YZPlane>(minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, minCorner.x, false, yAxisRotation, material)); // left
    sides.push_back(std::make_shared<YZPlane>(minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, maxCorner.x, true, yAxisRotation, material)); // right
}

float Box::intersect(const Ray& ray, glm::vec3& intersection) {
    std::shared_ptr<AxisAlignedPlane> closestSide;
    float closestIntersection = std::numeric_limits<float>::max();
    glm::vec3 closestIntersectionPoint;
    
    for (const std::shared_ptr<AxisAlignedPlane>& side : sides) {
        glm::vec3 intersectionPoint;
        float t = side->intersect(ray, intersectionPoint, offset);
        if (t > 0.0 && t < closestIntersection) {
            closestIntersection = t;
            closestIntersectionPoint = intersectionPoint;
            closestSide = side;
        }
    }
    
    if (closestIntersection < std::numeric_limits<float>::max()) {
        intersection = closestIntersectionPoint;
        intersectedSideNormal = closestSide->normal(closestIntersectionPoint);
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
