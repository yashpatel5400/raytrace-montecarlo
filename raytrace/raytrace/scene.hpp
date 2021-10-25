/**
 * @file scene.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef scene_hpp
#define scene_hpp

#include "geometry.hpp"
#include "material.hpp"
#include "util.hpp"

#include <vector>

struct Scene {
    std::vector<std::shared_ptr<Geometry>> geometry;
    Color backgroundColor;
    
    void addSphere(const glm::vec3& center, float radius, const std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<Sphere>(center, radius, material));
    }
    
    void addXYPlane(const float x1,
                    const float y1,
                    const float x2,
                    const float y2,
                    const float z,
                    std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<XYPlane>(x1, y1, x2, y2, z, material));
    }
    
    void addXZPlane(const float x1,
                    const float z1,
                    const float x2,
                    const float z2,
                    const float y,
                    std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<XYPlane>(x1, z1, x2, z2, y, material));
    }
    
    void addYZPlane(const float y1,
                    const float z1,
                    const float y2,
                    const float z2,
                    const float x,
                    std::shared_ptr<Material> material)  {
        geometry.push_back(std::make_shared<XYPlane>(y1, z1, y2, z2, x, material));
    }
};

Scene generateScene();

Color castRay(const Scene& scene, const Ray& ray, int bounce);

#endif /* scene_hpp */
