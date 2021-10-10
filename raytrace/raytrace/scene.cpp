/**
 * @file scene.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "scene.hpp"

#include "material.hpp"

#include <glm/gtc/random.hpp>

Scene generateScene() {
    Scene scene;
    
    scene.addSphere(glm::vec3(0.0, 0.0, -3.5), 0.5, std::make_shared<Lambertian>(PEACH));
    scene.addSphere(glm::vec3(2.2, 0.5, -2.5), 1.0, std::make_shared<Metal>(AQUA, 0.1));
    scene.addSphere(glm::vec3(-2.4, 0.3, -2.0), 0.8, std::make_shared<Dielectric>(1.5));
    scene.addSphere(glm::vec3(0.0, -1000.5, -2.0), 1000.0, std::make_shared<Lambertian>(BEIGE));

    const int kBallGridSize = 5;
    const float kBallRadius = 0.3;

    for (int a = -kBallGridSize; a < kBallGridSize; a++) {
        for (int b = -kBallGridSize; b < -1; b++) {
            glm::vec3 center(a + 0.9 * glm::linearRand(0.0f, 1.0f),
                             kBallRadius - 0.5,
                             b + 0.9 * glm::linearRand(0.0f, 1.0f));

            glm::vec3 randColor = glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
            float random = glm::linearRand(0.0f, 1.0f);
            if (random < 0.8) {
                scene.addSphere(center, kBallRadius, std::make_shared<Lambertian>(randColor));
            } else if (random < 0.95) {
                scene.addSphere(center, kBallRadius, std::make_shared<Metal>(randColor, glm::linearRand(0.0f, 1.0f)));
            } else {
                scene.addSphere(center, kBallRadius, std::make_shared<Dielectric>(1.5));
            }
        }
    }
    
    return scene;
}

// borrowed from https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
float intersectSphere(const Sphere& sphere, const Ray& ray) {
    glm::vec3 sphereToOrigin = ray.origin - sphere.center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0 * glm::dot(sphereToOrigin, ray.direction);
    float c = glm::dot(sphereToOrigin, sphereToOrigin) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
       return -1.0;
    }
    return (-b - sqrt(discriminant)) / (2.0 * a);
}

/**
 * returns color for the intersection of the ray with the scene. note that THIS is
 * where all the interesting Monte Carlo sampling will be happening!
 *
 */
Color findIntersection(const Scene& scene, const Ray& ray, int bounce) {
//    std::cout << bounce << std::endl;
    if (bounce < 0) {
        return BLACK;
    }
    
    Sphere closestObject;
    float closestIntersection = std::numeric_limits<float>::max();
    for (const Sphere& sphere : scene.spheres) {
        float intersection = intersectSphere(sphere, ray);
        if (intersection > 0 && intersection < closestIntersection) {
            closestIntersection = intersection;
            closestObject = sphere;
        }
    }
    
    if (closestIntersection < std::numeric_limits<float>::max()) {
        glm::vec3 intersectionPoint = ray.direction * closestIntersection;
        glm::vec3 normal = glm::normalize(intersectionPoint - closestObject.center);
        bool inside = glm::dot(ray.direction, normal) > 0;

        Ray scatteredRay;
        Color scatteredColor;
        bool didScatter = closestObject.material->scatter(ray,
                                                   intersectionPoint,
                                                   normal,
                                                   inside,
                                                   scatteredRay,
                                                   scatteredColor);
        if (!didScatter) {
            return BLACK;
        }
        return scatteredColor * findIntersection(scene, scatteredRay, bounce - 1);
    }
    
    float t = 0.5 * (ray.direction.y + 1.0);
    return glm::vec3(1.0, 1.0, 1.0) * (1.0f - t) + glm::vec3(0.5, 0.7, 1.0) * t;
}
