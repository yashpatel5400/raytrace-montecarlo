/**
 * @file scene.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef scene_hpp
#define scene_hpp

#include "material.hpp"
#include "util.hpp"

#include <vector>

struct Sphere {
    glm::vec3 center;
    float radius;
    std::shared_ptr<Material> material;
    
    Sphere() : radius(0), center(glm::vec3(0, 0, 0)) {}
    
    Sphere(const glm::vec3& center,
           float radius,
           const std::shared_ptr<Material> material) : center(center), radius(radius), material(material) {}
};

struct Scene {
    std::vector<Sphere> spheres;
    Color backgroundColor;
    
    void addSphere(const glm::vec3& center, float radius, const std::shared_ptr<Material> material) {
        spheres.push_back(Sphere(center, radius, material));
    }
};

Scene generateScene();

Color castRay(const Scene& scene, const Color& background, const Ray& ray, int bounce);

#endif /* scene_hpp */
