/**
 * @file material.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "material.hpp"

#include <glm/gtc/random.hpp>

// TODO: here is the other major spot for improving sampling from the BRDF function
glm::vec3 sampleUnitSphere() {
    while (true) {
        glm::vec3 proposal(
                           2.0 * (glm::linearRand(0.0f, 1.0f) - 0.5),
                           2.0 * (glm::linearRand(0.0f, 1.0f) - 0.5),
                           2.0 * (glm::linearRand(0.0f, 1.0f) - 0.5)
                           );
        if (glm::length(proposal) <= 1.0) {
            return proposal;
        }
    }
}

Lambertian::Lambertian(const Color& texture) : texture(texture) {}
    
const bool Lambertian::scatter(const Ray& in,
                               const glm::vec3& intersection,
                               const glm::vec3& normal,
                               const bool inside,
                               Ray& out,
                               Color& outColor) const {
    glm::vec3 outDirection = glm::normalize(normal + sampleUnitSphere());
    
    // edge case that we should avoid
    const float kSmidgen = 1e-5;
    if (fabs(outDirection.x) < kSmidgen && fabs(outDirection.y) < kSmidgen && fabs(outDirection.z) < kSmidgen) {
        outDirection = normal;
    }
    
    out = Ray(outDirection, intersection);
    outColor = texture;
    return true;
}

Metal::Metal(const Color& texture, float roughness) : texture(texture), roughness(roughness) {}
    
const bool Metal::scatter(const Ray& in,
                   const glm::vec3& intersection,
                   const glm::vec3& normal,
                   const bool inside,
                   Ray& out,
                   Color& outColor) const {
    glm::vec3 outDirection = glm::reflect(in.direction, normal);
    // have some degree of scattering in the BRDF if material is modelled as being rough
    outDirection += sampleUnitSphere() * roughness;
    outDirection = glm::normalize(outDirection);
    
    out = Ray(outDirection, intersection);
    outColor = texture;
    return true;
}

Dielectric::Dielectric(const float ior) : ior(ior) {}

const bool Dielectric::scatter(const Ray& in,
                   const glm::vec3& intersection,
                   const glm::vec3& normal,
                   const bool inside,
                   Ray& out,
                   Color& outColor) const {
    float cosTheta = glm::dot(-in.direction, normal);
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    
    float eta = !inside ? 1.0 / ior : ior;
    
    // Schlick's approximation to determine whether we want to reflect or refract
    float r0 = (1 - eta) / (1 + eta);
    float r02 = r0 * r0;
    float rTheta = r02 + (1 - r02) * pow((1 - cosTheta), 5.0);
    float random = glm::linearRand(0.0f, 1.0f);
    
    glm::vec3 outDirection = sinTheta * eta > 1.0 || random < rTheta
        ? glm::reflect(in.direction, normal)
        : glm::refract(in.direction, normal, eta);
    out = Ray(outDirection, intersection);
    outColor = WHITE;
    return true;
}
