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

const int centerZ = -1500;

const int sizeX = 500;
const int sizeY = 500;
const int sizeZ = 250;

Scene generateCornellBoxScene() {
    Scene scene;

    scene.addXYPlane(-sizeX, -sizeY, sizeX, sizeY, centerZ - sizeZ, true, 0.0, std::make_shared<Lambertian>(WHITE)); // back
    scene.addYZPlane(-sizeY, centerZ - sizeZ, sizeY, centerZ + sizeZ, -sizeX, true, 0.0, std::make_shared<Lambertian>(GREEN)); // left
    scene.addYZPlane(-sizeY, centerZ - sizeZ, sizeY, centerZ + sizeZ, sizeX, false, 0.0, std::make_shared<Lambertian>(RED)); // right
    scene.addXZPlane(-sizeX, centerZ - sizeZ, sizeX, centerZ + sizeZ, -sizeY, true, 0.0, std::make_shared<Lambertian>(WHITE)); // bottom
    scene.addXZPlane(-sizeX, centerZ - sizeZ, sizeX, centerZ + sizeZ, sizeY, false, 0.0, std::make_shared<Lambertian>(WHITE)); // top
    
    scene.addXZPlane(-sizeX / 2.0, centerZ - sizeZ / 2.0,
                     sizeX / 2.0, centerZ + sizeZ / 2.0, sizeY - .005, true, 0.0, std::make_shared<Light>(LIGHT_GRAY)); // on ceilling
    
    scene.addBox(glm::vec3(550.0 + -sizeX / 3.0, -sizeY + 0.01, 10.0 + centerZ - sizeZ / 3.0),
                 glm::vec3(550.0 + sizeX / 3.0, 1.0 * sizeY / 5.0, 10.0 + centerZ + sizeZ / 3.0),
                 0.45,
                 std::make_shared<Lambertian>(WHITE));
//    scene.addBox(glm::vec3(-650.0 + -sizeX / 4.0, -sizeY + 0.01, 225.0 + centerZ - sizeZ / 4.0),
//                 glm::vec3(-650.0 + sizeX / 4.0, -2.0 * sizeY / 5.0, 225.0 + centerZ + sizeZ / 4.0),
//                 -0.55,
//                 std::make_shared<Lambertian>(WHITE));
    scene.addSphere(
                    glm::vec3(175.0, -3.0 * sizeY / 5.0, 200.0 + centerZ - sizeZ / 4.0),
                    200.0, std::make_shared<Dielectric>(1.5));
    
    scene.backgroundColor = BLACK;
    
    return scene;
}

void populateClosestIntersection(const Scene& scene,
                                 const Ray& ray,
                                 std::shared_ptr<Geometry>& closestObject,
                                 float& closestIntersection,
                                 glm::vec3& closestIntersectionPoint) {
    for (const std::shared_ptr<Geometry>& geometry : scene.geometry) {
        glm::vec3 intersectionPoint;
        float intersection = geometry->intersect(ray, intersectionPoint);
        if (intersection > 0 && intersection < closestIntersection) {
            closestIntersection = intersection;
            closestIntersectionPoint = intersectionPoint;
            closestObject = geometry;
        }
    }
}

/**
 * returns color for the intersection of the ray with the scene. note that THIS is
 * where all the interesting Monte Carlo sampling will be happening!
 *
 */
Color castRay(const Scene& scene, const Ray& ray, int bounce) {
    if (bounce < 0) {
        return BLACK;
    }
    
    std::shared_ptr<Geometry> closestObject;
    float closestIntersection = std::numeric_limits<float>::max();
    glm::vec3 closestIntersectionPoint;
    populateClosestIntersection(scene, ray, closestObject, closestIntersection, closestIntersectionPoint);
    
    if (closestIntersection < std::numeric_limits<float>::max()) {
        glm::vec3 normal = closestObject->normal(closestIntersectionPoint);
        bool inside = glm::dot(ray.direction, normal) > 0;

        Ray scatteredRay;
        Color scatteredColor;
        Color emissionColor = closestObject->material->emit(closestIntersectionPoint, normal);
        double pdf;
        
        bool didScatter = closestObject->material->scatter(ray,
                                                           closestIntersectionPoint,
                                                           normal,
                                                           inside,
                                                           scatteredRay,
                                                           scatteredColor,
                                                           pdf);
        if (!didScatter) {
            return emissionColor;
        }
        
        // recall: E_{X ~ P}[A * color * (s / P)] is an MIS estimate w/ sampling distribution P and scatter S
        // this equation maps exactly to this line of code, with scatterPDF being S and pdf being P
        
        // if P == 0, that means the scattering distribution has not been defined for that material, so we *don't*
        // do MIS in that case and just use standard sampling
        if (pdf == 0.0) {
            return emissionColor + scatteredColor * castRay(scene, scatteredRay, bounce - 1);
        }
        
        return emissionColor + scatteredColor * castRay(scene, scatteredRay, bounce - 1) *
            static_cast<float>(closestObject->material->scatterPDF(normal, scatteredRay.direction) / pdf);
    }
    
    return scene.backgroundColor;
}
