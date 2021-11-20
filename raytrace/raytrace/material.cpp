/**
 * @file material.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "material.hpp"

#include <iostream>
#include "math.h"
#include <glm/gtc/random.hpp>

// can be modified for arbitrary choice of function for sampling about z-axis
glm::vec3 uniformlySampleHemisphere() {
    float r1 = glm::linearRand(0.0f, 1.0f);
    float r2 = glm::linearRand(0.0f, 1.0f);
    
    float phi = 2 * M_PI * r1;
    
    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);
    float z = sqrt(1 - r2);
    return glm::vec3(x, y, z);
}

// taken from https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#onedimensionalmcintegration
glm::mat3 localCoordSystem(const glm::vec3& n) {
    glm::vec3 normal = glm::normalize(n);
    glm::vec3 a = (fabs(normal.x) > 0.9) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    
    glm::vec3 z = normal;
    glm::vec3 y = glm::normalize(glm::cross(normal, a));
    glm::vec3 x = glm::cross(z, y);
    
    glm::mat3 transformer;
    transformer[0] = x;
    transformer[1] = y;
    transformer[2] = z;
    return transformer;
}

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
                               Color& outColor,
                               double& pdf) const {
    // need to do change of basis to do sampling from out of the normal of intersection
    glm::mat3 localBasis = localCoordSystem(normal);
    glm::vec3 globalRandomDirection = uniformlySampleHemisphere();
    glm::vec3 outDirection = glm::normalize(localBasis * globalRandomDirection);
    
    // edge case that we should avoid
    const float kSmidgen = 1e-5;
    if (fabs(outDirection.x) < kSmidgen && fabs(outDirection.y) < kSmidgen && fabs(outDirection.z) < kSmidgen) {
        outDirection = normal;
    }
    
    out = Ray(outDirection, intersection);
    outColor = texture;
    pdf = glm::dot(localBasis[2], outDirection) / M_PI; // PDF of *sampling* PDF (NOT necessarily scatter PDF)
    return true;
}

double Lambertian::scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
    float cos = glm::dot(glm::normalize(normal), glm::normalize(outDirection));
    return fmax(0.001, cos / M_PI);
}

Metal::Metal(const Color& texture, float roughness) : texture(texture), roughness(roughness) {}
    
const bool Metal::scatter(const Ray& in,
                          const glm::vec3& intersection,
                          const glm::vec3& normal,
                          const bool inside,
                          Ray& out,
                          Color& outColor,
                          double& pdf) const {
    glm::vec3 outDirection = glm::reflect(in.direction, normal);
    // have some degree of scattering in the BRDF if material is modelled as being rough
    outDirection += sampleUnitSphere() * roughness;
    outDirection = glm::normalize(outDirection);
    
    out = Ray(outDirection, intersection);
    outColor = texture;
    pdf = scatterPDF(in.direction, outDirection);
    return glm::dot(outDirection, normal) > 0;
}

double Metal::scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
    return 0;
}

Dielectric::Dielectric(const float ior) : ior(ior) {}

const bool Dielectric::scatter(const Ray& in,
                               const glm::vec3& intersection,
                               const glm::vec3& normal,
                               const bool inside,
                               Ray& out,
                               Color& outColor,
                               double& pdf) const {
    float cosTheta = fmin(glm::dot(-in.direction, normal), 1.0);
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    
    float eta = inside ? ior : 1.0 / ior;
    
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
    pdf = scatterPDF(in.direction, outDirection);
    return true;
}

double Dielectric::scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
    return 0;
}

Light::Light(const Color& texture) : texture(texture) {}

const bool Light::scatter(const Ray& in,
                          const glm::vec3& intersection,
                          const glm::vec3& normal,
                          const bool inside,
                          Ray& out,
                          Color& outColor,
                          double& pdf) const {
    return false; // light sources do not have scattering effects
}

Color Light::emit(const glm::vec3& intersection) const {
    return texture;
}

double Light::scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
    return 0;
}
