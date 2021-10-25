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

struct AxisAlignedPlane {
    float x1, y1, x2, y2, z; // coordinates (currently assuming axis aligned)
    std::shared_ptr<Material> material;
    
    AxisAlignedPlane() : x1(0), y1(0), x2(0), y2(0), z(0), material(material) {}
    
    AxisAlignedPlane(const float x1,
                     const float y1,
                     const float x2,
                     const float y2,
                     const float z,
                     std::shared_ptr<Material> material) : x1(x1), y1(y1), x2(x2), y2(y2), z(z), material(material) {}
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<AxisAlignedPlane> planes;
    Color backgroundColor;
    
    void addSphere(const glm::vec3& center, float radius, const std::shared_ptr<Material> material) {
        spheres.push_back(Sphere(center, radius, material));
    }
    
    void addAxisAlignedPlane(const float x1,
                             const float y1,
                             const float x2,
                             const float y2,
                             const float z,
                             std::shared_ptr<Material> material) {
        planes.push_back(AxisAlignedPlane(x1, y1, x2, y2, z, material));
    }
};

Scene generateScene();

Color castRay(const Scene& scene, const Ray& ray, int bounce);

#endif /* scene_hpp */
