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
                    const bool facingAxis,
                    const float yAxisRotation,
                    std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<XYPlane>(x1, y1, x2, y2, z, facingAxis, yAxisRotation, material));
    }
    
    void addXZPlane(const float x1,
                    const float z1,
                    const float x2,
                    const float z2,
                    const float y,
                    const bool facingAxis,
                    const float yAxisRotation,
                    std::shared_ptr<Material> material) {
        geometry.push_back(std::make_shared<XZPlane>(x1, z1, x2, z2, y, facingAxis, yAxisRotation, material));
    }
    
    void addYZPlane(const float y1,
                    const float z1,
                    const float y2,
                    const float z2,
                    const float x,
                    const bool facingAxis,
                    const float yAxisRotation,
                    std::shared_ptr<Material> material)  {
        geometry.push_back(std::make_shared<YZPlane>(y1, z1, y2, z2, x, facingAxis, yAxisRotation, material));
    }
    
    void addBox(const glm::vec3& minCorner,
                const glm::vec3& maxCorner,
                const float yAxisRotation,
                std::shared_ptr<Material> material)  {
        geometry.push_back(std::make_shared<Box>(minCorner, maxCorner, yAxisRotation, material));
    }
};

Scene generateBallScene();
Scene generateCornellBoxScene();

Color castRay(const Scene& scene, const Ray& ray, int bounce);
void populateClosestIntersection(const Scene& scene,
                                const Ray& ray,
                                std::shared_ptr<Geometry>& closestObject,
                                float& closestIntersection,
                                glm::vec3& closestIntersectionPoint);

#endif /* scene_hpp */
