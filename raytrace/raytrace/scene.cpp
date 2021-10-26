/**
 * @file scene.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "scene.hpp"

#include "material.hpp"

#include <iostream>

#include <glm/gtc/random.hpp>

Scene generateBallScene() {
    Scene scene;
    
    scene.addSphere(glm::vec3(0.0, 0.0, -3.5), 0.5, std::make_shared<Lambertian>(PEACH));
    scene.addSphere(glm::vec3(2.2, 0.5, -2.5), 1.0, std::make_shared<Metal>(AQUA, 0.1));
    scene.addSphere(glm::vec3(-1.6, 0.3, -2.0), 0.8, std::make_shared<Dielectric>(1.5));
    scene.addSphere(glm::vec3(0.0, -1000.5, -2.0), 1000.0, std::make_shared<Lambertian>(BEIGE));

    const int kBallGridSize = 5;
    const float kBallRadius = 0.2;

    for (int a = -kBallGridSize; a < kBallGridSize; a++) {
        for (int b = -kBallGridSize; b < kBallGridSize - 1; b++) {
            const float ballRadius = kBallRadius * glm::linearRand(0.5f, 1.0f);
            glm::vec3 center(a * 0.75 + 0.9 * glm::linearRand(0.0f, 1.0f),
                             ballRadius - 0.5,
                             b * 0.75 + 0.9 * glm::linearRand(0.0f, 1.0f));

            glm::vec3 randColor = glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
            float random = glm::linearRand(0.0f, 1.0f);
            if (random < 0.8) {
                scene.addSphere(center, ballRadius, std::make_shared<Lambertian>(randColor));
            } else if (random < 0.95) {
                scene.addSphere(center, ballRadius, std::make_shared<Metal>(randColor, glm::linearRand(0.0f, 1.0f)));
            } else {
                scene.addSphere(center, ballRadius, std::make_shared<Dielectric>(1.5));
            }
        }
    }
    
    scene.backgroundColor = BLACK;
    
    return scene;
}

Scene generateCornellBoxScene() {
    Scene scene;
    
    const int centerZ = -107;
    
    const int sizeX = 50;
    const int sizeY = 50;
    const int sizeZ = 50;

    scene.addXYPlane(-sizeX, -sizeY, sizeX, sizeY, centerZ - sizeZ, std::make_shared<Lambertian>(WHITE)); // back
    scene.addYZPlane(-sizeY, centerZ - sizeZ, sizeY, centerZ + sizeZ, -sizeX, std::make_shared<Lambertian>(GREEN)); // left
    scene.addYZPlane(-sizeY, centerZ - sizeZ, sizeY, centerZ + sizeZ, sizeX, std::make_shared<Lambertian>(RED)); // right
    scene.addXZPlane(-sizeX, centerZ - sizeZ, sizeX, centerZ + sizeZ, -sizeY, std::make_shared<Lambertian>(WHITE)); // bottom
    scene.addXZPlane(-sizeX, centerZ - sizeZ, sizeX, centerZ + sizeZ, sizeY, std::make_shared<Lambertian>(WHITE)); // top
    
    scene.addXZPlane(-sizeX / 4.0f, centerZ - sizeZ / 4.0f,
                     sizeX / 4.0f, centerZ + sizeZ / 4.0f, sizeY - 0.025, std::make_shared<Light>(LIGHT_GRAY)); // on ceilling
    
    scene.backgroundColor = BLACK;
    
    return scene;
}

/**
 * returns color for the intersection of the ray with the scene. note that THIS is
 * where all the interesting Monte Carlo sampling will be happening!
 *
 */
Color castRay(const Scene& scene, const Ray& ray, int bounce) {
//    std::cout << bounce << std::endl;
    if (bounce < 0) {
        return BLACK;
    }
    
    std::shared_ptr<Geometry> closestObject;
    float closestIntersection = std::numeric_limits<float>::max();
    for (const std::shared_ptr<Geometry>& geometry : scene.geometry) {
        float intersection = geometry->intersect(ray);
        if (intersection > 0 && intersection < closestIntersection) {
            closestIntersection = intersection;
            closestObject = geometry;
        }
    }
    
    if (closestIntersection < std::numeric_limits<float>::max()) {
        glm::vec3 intersectionPoint = ray.direction * closestIntersection;
        glm::vec3 normal = closestObject->normal(intersectionPoint);
        bool inside = glm::dot(ray.direction, normal) > 0;

        Ray scatteredRay;
        Color scatteredColor;
        Color emissionColor = closestObject->material->emit(intersectionPoint);
        
        bool didScatter = closestObject->material->scatter(ray,
                                                   intersectionPoint,
                                                   normal,
                                                   inside,
                                                   scatteredRay,
                                                   scatteredColor);

        if (!didScatter) {
            return emissionColor;
        }
        return emissionColor + scatteredColor * castRay(scene, scatteredRay, bounce - 1);
    }
    
    return scene.backgroundColor;
}
