/**
 * @file geometry.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef geometry_hpp
#define geometry_hpp

#include "material.hpp"
#include "util.hpp"

struct Geometry {
    std::shared_ptr<Material> material;
    
    Geometry() {}
    Geometry(std::shared_ptr<Material> material) : material(material) {}
    
    virtual float intersect(const Ray& ray, glm::vec3& intersection) = 0;
    virtual glm::vec3 normal(const glm::vec3& intersectionPoint) = 0;
};

struct Sphere : public Geometry {
    glm::vec3 center;
    float radius;
    
    Sphere() : radius(0), center(glm::vec3(0, 0, 0)) {}
    
    Sphere(const glm::vec3& center,
           float radius,
           const std::shared_ptr<Material> material) : center(center), radius(radius), Geometry(material) {}
    
    float intersect(const Ray& ray, glm::vec3& intersection) override;
    glm::vec3 normal(const glm::vec3& intersectionPoint) override;
};

struct AxisAlignedPlane : public Geometry {
    float varAxis11, varAxis21, varAxis12, varAxis22;
    float constAxis;
    int varAxis1Index, varAxis2Index, constAxisIndex;
    bool facingAxis;
    float yAxisRotation;
    
    AxisAlignedPlane() : varAxis11(0), varAxis21(0), varAxis12(0), varAxis22(0), constAxis(0),
                         varAxis1Index(0), varAxis2Index(0), constAxisIndex(0) {}
    
    AxisAlignedPlane(const float varAxis11,
                     const float varAxis21,
                     const float varAxis12,
                     const float varAxis22,
                     const float constAxis,
                     const int varAxis1Index,
                     const int varAxis2Index,
                     const int constAxisIndex,
                     const bool facingAxis, // determines direction of normal
                     const float yAxisRotation,
                     std::shared_ptr<Material> material) :
                        varAxis11(varAxis11),
                        varAxis21(varAxis21),
                        varAxis12(varAxis12),
                        varAxis22(varAxis22),
                        constAxis(constAxis),
                        varAxis1Index(varAxis1Index),
                        varAxis2Index(varAxis2Index),
                        constAxisIndex(constAxisIndex),
                        facingAxis(facingAxis),
                        yAxisRotation(yAxisRotation),
                        Geometry(material) {}
    
    float intersect(const Ray& ray, glm::vec3& intersection) override;
    glm::vec3 normal(const glm::vec3& intersectionPoint) override;
};

struct XYPlane : public AxisAlignedPlane {
    XYPlane(const float x1,
            const float y1,
            const float x2,
            const float y2,
            const float z,
            const bool facingAxis, // determines direction of normal
            const float yAxisRotation,
            std::shared_ptr<Material> material) : AxisAlignedPlane(x1, y1, x2, y2, z, 0, 1, 2, facingAxis, yAxisRotation, material) {}
};

struct XZPlane : public AxisAlignedPlane {
    XZPlane(const float x1,
            const float z1,
            const float x2,
            const float z2,
            const float y,
            const bool facingAxis, // determines direction of normal
            const float yAxisRotation,
            std::shared_ptr<Material> material) : AxisAlignedPlane(x1, z1, x2, z2, y, 0, 2, 1, facingAxis, yAxisRotation, material) {}
};

struct YZPlane : public AxisAlignedPlane {
    YZPlane(const float y1,
            const float z1,
            const float y2,
            const float z2,
            const float x,
            const bool facingAxis, // determines direction of normal
            const float yAxisRotation,
            std::shared_ptr<Material> material) : AxisAlignedPlane(y1, z1, y2, z2, x, 1, 2, 0, facingAxis, yAxisRotation, material) {}
};


struct Box : public Geometry {
    std::vector<std::shared_ptr<AxisAlignedPlane>> sides;
    glm::vec3 intersectedSideNormal;
    
    Box(const glm::vec3& minCorner,
        const glm::vec3& maxCorner,
        const float yAxisRotation,
        std::shared_ptr<Material> material);
    
    float intersect(const Ray& ray, glm::vec3& intersection) override;
    glm::vec3 normal(const glm::vec3& intersectionPoint) override;
};

#endif /* geometry_hpp */
