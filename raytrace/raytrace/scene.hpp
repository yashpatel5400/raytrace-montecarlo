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

struct Geometry {
    std::shared_ptr<Material> material;
    
    Geometry() {}
    Geometry(std::shared_ptr<Material> material) : material(material) {}
    
    virtual float intersect(const Ray& ray) = 0;
    virtual glm::vec3 normal(const glm::vec3& intersectionPoint) = 0;
};

struct Sphere : public Geometry {
    glm::vec3 center;
    float radius;
    
    Sphere() : radius(0), center(glm::vec3(0, 0, 0)) {}
    
    Sphere(const glm::vec3& center,
           float radius,
           const std::shared_ptr<Material> material) : center(center), radius(radius), Geometry(material) {}
    
    float intersect(const Ray& ray) override;
    glm::vec3 normal(const glm::vec3& intersectionPoint) override;
};

struct AxisAlignedPlane : public Geometry {
    float x1, y1, x2, y2, z; // coordinates (currently assuming axis aligned)
    
    AxisAlignedPlane() : x1(0), y1(0), x2(0), y2(0), z(0) {}
    
    AxisAlignedPlane(const float x1,
                     const float y1,
                     const float x2,
                     const float y2,
                     const float z,
                     std::shared_ptr<Material> material) : x1(x1), y1(y1), x2(x2), y2(y2), z(z), Geometry(material) {}
    
    float intersect(const Ray& ray) override;
    glm::vec3 normal(const glm::vec3& intersectionPoint) override;
};

struct Scene {
    std::vector<std::shared_ptr<Geometry>> geometry;
    Color backgroundColor;
    
    void addSphere(const glm::vec3& center, float radius, const std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<Sphere>(center, radius, material));
    }
    
    void addAxisAlignedPlane(const float x1,
                             const float y1,
                             const float x2,
                             const float y2,
                             const float z,
                             std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<AxisAlignedPlane>(x1, y1, x2, y2, z, material));
    }
};

Scene generateScene();

Color castRay(const Scene& scene, const Ray& ray, int bounce);

#endif /* scene_hpp */
