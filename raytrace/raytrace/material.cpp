/**
 * @file material.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "material.hpp"


#include "scene.hpp"

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

// can be modified for arbitrary choice of function for sampling about z-axis
glm::vec3 uniformlySampleSphere(const float radius, const float dist_sq) {
    float r1 = glm::linearRand(0.0f, 1.0f);
    float r2 = glm::linearRand(0.0f, 1.0f);
    
    float z = 1 + r2 * (sqrt(1 - radius * radius / dist_sq) - 1);
    float phi = 2 * M_PI * r1;
    
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);

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

const int centerZ = -1500;

const int sizeX = 500;
const int sizeY = 500;
const int sizeZ = 250;

float computeLightPDF(const Ray& outbound) {
    // need to determine whether the ray intersect the light (if not, 0 PDF)
    const std::shared_ptr<XZPlane> light = std::make_shared<XZPlane>(-sizeX / 2.0, centerZ - sizeZ / 2.0,
                                                                     sizeX / 2.0, centerZ + sizeZ / 2.0,
                                                                     sizeY - .005, true, 0.0, std::make_shared<Light>(LIGHT_GRAY));
    glm::vec3 intersectionPoint;
    float intersection = light->intersect(outbound, intersectionPoint);
    if (intersection < 0) {
        return 0.0;
    }
    
    glm::vec3 dirToLight = intersectionPoint - outbound.origin;
    float distToLight2 = glm::dot(dirToLight, dirToLight);
    dirToLight = glm::normalize(dirToLight);
    float lightArea = 125000; // TODO: hardcoded light area! bleh
    float cosineAlpha = fabs(dirToLight.y);
    
    return distToLight2 / (cosineAlpha * lightArea);
}

float computeSpherePDF(const Ray& outbound) {
    const std::shared_ptr<Sphere> glassBall = std::make_shared<Sphere>(glm::vec3(175.0, -3.0 * sizeY / 5.0, 200.0 + centerZ - sizeZ / 4.0),
                                                                       200.0, std::make_shared<Dielectric>(1.5));
    glm::vec3 intersectionPoint;
    float intersection = glassBall->intersect(outbound, intersectionPoint);
    if (intersection < 0) {
        return 0.0;
    }
    
    glm::vec3 dirToSphere = glassBall->center - outbound.origin;
    float distToSphere2 = glm::dot(dirToSphere, dirToSphere);
    
    const float ratio = glassBall->radius * glassBall->radius / distToSphere2;
    float cosThetaMax = sqrt(1 - ratio);
    float solidAngle = 2 * M_PI * (1 - cosThetaMax);

    return 1.0f / solidAngle;
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
    /* ***********************************************************************
     * Brief Interlude: Monte Carlo Importance Sampling
     * -----------------------------------------------------------------------
     * The next chunk of code is the *crux of the Monte Carlo importance sampling*
     * in ray tracing. A quick explanation will probably help explain how it works:
     *
     * When we want to calculate E_X[A * s * color], we can choose any distribution
     * for X ~ P, with the initial obvious choice being s. Instead, any other
     * distribution *can* be used and often will totally change the variance profile
     * (and therefore the convergence properties). In particular, we want to sample
     * *more* from the light sources, since those are the most important directions
     * for the final value of the color.
     *
     * The issue with just changing the sampling of the light rays directly is that
     * (if we do this without changing anything else), the result is no longer
     * guaranteed to converge with enough samples (!) So, we have to normalize it
     * by the PDF of the sampling we take. Finding this pdf is mathematically rather
     * simple, but an absolutely critical step to actually get the correct answer.
     *
     * The key realization is that linear interpolations a_i of a family of PDFs
     * {f_i} in the form f(x) = \sum a_i * f_i(x) is itself a PDF and is precisel
     * the PDF of interest if you are sampling with some probability from the light
     * sources! This is what is actually defined in the function above and used
     * in the following lines of code. It may seem simple (and the code is!) but the
     * conceptual idea that underlies this is surprisingly involved and is belied
     * by the seeming simplicity of the code.
     *
     * The PDF for rectangular light sources turns out to be simple: d(p,q)^2 / (cos(theta) * A)
     * *********************************************************************** */
    glm::vec3 outDirection;
    
    // TODO: this is a TOTAL hack to get around the firefly issues seen in the renders -- unclear what the cause is
    const float kFireflyPdfThresh = 0.15;
    while (pdf < kFireflyPdfThresh) {
        std::vector<float> alphas = { .5, 0.0 } ; // mixing between light, sphere, and (implicit rest) random
        const float randSampling = glm::linearRand(0.0f, 1.0f);
        if (randSampling < alphas[0]) {
            glm::vec3 randomLightPoint(
                                       glm::linearRand(-sizeX / 2.0, sizeX / 2.0),
                                       sizeY - .005,
                                       glm::linearRand(centerZ - sizeZ / 2.0, centerZ + sizeZ / 2.0)
            );
            outDirection = glm::normalize(randomLightPoint - intersection);
        }
        else if (randSampling < alphas[1]) {
            glm::vec3 sphereCenter = glm::vec3(175.0, -3.0 * sizeY / 5.0, 200.0 + centerZ - sizeZ / 4.0);
            const float sphereRadius = 200.0;
            glm::vec3 directionToCenter = sphereCenter - intersection;
            float sphereDistanceSq = glm::dot(directionToCenter, directionToCenter);
            directionToCenter = glm::normalize(directionToCenter);
            
            glm::mat3 localBasis = localCoordSystem(directionToCenter);
            glm::vec3 globalRandomDirection = uniformlySampleSphere(sphereRadius, sphereDistanceSq);
            outDirection = glm::normalize(localBasis * globalRandomDirection);
        }
        else {
            // need to do change of basis to do sampling from out of the normal of intersection
            glm::mat3 localBasis = localCoordSystem(normal);
            glm::vec3 globalRandomDirection = uniformlySampleHemisphere();
            outDirection = glm::normalize(localBasis * globalRandomDirection);
            // edge case that we should avoid
            const float kSmidgen = 1e-5;
            if (fabs(outDirection.x) < kSmidgen && fabs(outDirection.y) < kSmidgen && fabs(outDirection.z) < kSmidgen) {
                outDirection = normal;
            }
        }
        
        out = Ray(outDirection, intersection);
        outColor = texture;
        
        float lightPDF = computeLightPDF(out);
        float spherePDF = computeSpherePDF(out);
        float hemispherePDF = glm::dot(normal, outDirection) / M_PI; // PDF of *sampling* PDF (NOT necessarily scatter PDF)
    //    pdf = alphas[0] * lightPDF + (alphas[1] - alphas[0]) * spherePDF + (1 - alphas[1]) * hemispherePDF;
        pdf = alphas[0] * lightPDF + (1 - alphas[0]) * hemispherePDF;
    }
    
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
    
    bool doReflection = sinTheta * eta > 1.0 || random < rTheta;
    glm::vec3 outDirection = doReflection
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

Color Light::emit(const glm::vec3& intersection, const glm::vec3& normal) const {
    // TODO: hard-coded normal for unidirectional light forces lights to all be y axis aligned
    if (normal.y < 0) {
        return Color(0, 0, 0);
    }
    return texture;
}

double Light::scatterPDF(const glm::vec3& normal, const glm::vec3& outDirection) const {
    return 0;
}
