/**
 * @file material.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef material_hpp
#define material_hpp

#include "util.hpp"

#define WHITE Color(1.00, 1.00, 1.00)
#define SILVER Color(.75, .75, .75)
#define GRAY Color(.50, .50, .50)
#define BLACK Color(0, 0, 0)
#define RED Color(1.00, 0, 0)
#define MAROON Color(.50, 0, 0)
#define YELLOW Color(1.00, 1.00, 0)
#define OLIVE Color(.50, .50, 0)
#define LIME Color(0, 1.00, 0)
#define GREEN Color(0, .50, 0)
#define AQUA Color(0, 1.00, 1.00)
#define TEAL Color(0, .50, .50)
#define BLUE Color(0, 0, 1.00)
#define NAVY Color(0, 0, .50)
#define FUCHSIA Color(1.00, 0, 1.00)
#define PURPLE Color(.50, 0, .50)
#define PEACH Color(0.7, 0.3, 0.3)
#define LIGHT_GRAY Color(0.8, 0.8, 0.8)
#define BEIGE Color(0.8, 0.6, 0.2)

// materials are characterized by their BRDF/BDTF, so these abstract methods are left to implementations
struct Material {
    // returns whether or not a scatter happened (could have been absorbed) and populates out ray/color if so
    virtual const bool scatter(const Ray& in,
                               const glm::vec3& intersection,
                               const glm::vec3& normal,
                               const bool inside,
                               Ray& out,
                               Color& outColor,
                               double& pdf) const = 0;
    
    virtual Color emit(const glm::vec3& intersection, const glm::vec3& normal) const {
        return Color(0, 0, 0);
    }
    
    virtual double scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
        return 0;
    }
};

struct Lambertian : public Material {
    Lambertian(const Color& texture);
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       Color& outColor,
                       double& pdf) const override;
    double scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const override;
    
    Color texture;
};

struct Metal : public Material {
    Metal(const Color& texture, float roughness);
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       Color& outColor,
                       double& pdf) const override;
    double scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const override;
    
    Color texture;
    float roughness;
};

struct Dielectric : public Material {
    Dielectric(const float ior);
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       Color& outColor,
                       double& pdf) const override;
    double scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const override;
    
    float ior;
};

struct Light : public Material {
    Light(const Color& texture);
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       Color& outColor,
                       double& pdf) const override;
    double scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const override;
    
    Color emit(const glm::vec3& intersection, const glm::vec3& normal) const override;

    Color texture;
};

#endif /* material_hpp */
