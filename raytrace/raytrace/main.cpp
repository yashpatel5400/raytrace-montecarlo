/**
 * @file main.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include <fstream>
#include <iostream>

#include <vector>

#include <gflags/gflags.h>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/string_cast.hpp>

DEFINE_string(filename, "", "Output file for rendering");
DEFINE_int32(width, 0, "Width of rendering");
DEFINE_int32(height, 0, "Height of rendering");
DEFINE_int32(samples, 5, "Number of samples per pixel");
DEFINE_int32(bounces, 1, "Depth of bounces");

using color = glm::vec3;

#define WHITE color(1.00, 1.00, 1.00)
#define SILVER color(.75, .75, .75)
#define GRAY color(.50, .50, .50)
#define BLACK color(0, 0, 0)
#define RED color(1.00, 0, 0)
#define MAROON color(.50, 0, 0)
#define YELLOW color(1.00, 1.00, 0)
#define OLIVE color(.50, .50, 0)
#define LIME color(0, 1.00, 0)
#define GREEN color(0, .50, 0)
#define AQUA color(0, 1.00, 1.00)
#define TEAL color(0, .50, .50)
#define BLUE color(0, 0, 1.00)
#define NAVY color(0, 0, .50)
#define FUCHSIA color(1.00, 0, 1.00)
#define PURPLE color(.50, 0, .50)
#define PEACH color(0.7, 0.3, 0.3)
#define LIGHT_GRAY color(0.8, 0.8, 0.8)
#define BEIGE color(0.8, 0.6, 0.2)

struct Ray {
    glm::vec3 direction;
    glm::vec3 origin;
    
    Ray() {
        direction = glm::vec3(0, 0, 0);
        origin = glm::vec3(0, 0, 0);
    }
    
    Ray(const glm::vec3& direction,
           const glm::vec3& origin) : direction(direction), origin(origin) {}
};

// TODO: here is the other major spot for improving sampling from the BRDF function
glm::vec3 sampleUnitSphere() {
    while (true) {
        glm::vec3 proposal(
                           2.0 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5),
                           2.0 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5),
                           2.0 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5)
                           );
        if (glm::length(proposal) <= 1.0) {
            return proposal;
        }
    }
}

// materials are characterized by their BRDF/BDTF, so these abstract methods are left to implementations
struct Material {
    // returns whether or not a scatter happened (could have been absorbed) and populates out ray/color if so
    virtual const bool scatter(const Ray& in,
                               const glm::vec3& intersection,
                               const glm::vec3& normal,
                               const bool inside,
                               Ray& out,
                               color& outColor) const = 0;
};

struct Lambertian : public Material {
    color texture;

    Lambertian(const color& texture) : texture(texture) {}
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       color& outColor) const override {
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
};

struct Metal : public Material {
    color texture;
    float roughness;

    Metal(const color& texture, float roughness) : texture(texture), roughness(roughness) {}
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       color& outColor) const override {
        glm::vec3 outDirection = glm::reflect(in.direction, normal);
        // have some degree of scattering in the BRDF if material is modelled as being rough
        outDirection += sampleUnitSphere() * roughness;
        outDirection = glm::normalize(outDirection);
        
        out = Ray(outDirection, intersection);
        outColor = texture;
        return true;
    }
};

struct Dielectric : public Material {
    float ior;
    
    Dielectric(const float ior) : ior(ior) {}
    
    const bool scatter(const Ray& in,
                       const glm::vec3& intersection,
                       const glm::vec3& normal,
                       const bool inside,
                       Ray& out,
                       color& outColor) const override {
        float cosTheta = glm::dot(-in.direction, normal);
        float sinTheta = sqrt(1 - cosTheta * cosTheta);
        
        float eta = !inside ? 1.0 / ior : ior;
        
        // Schlick's approximation to determine whether we want to reflect or refract
        float r0 = (1 - eta) / (1 + eta);
        float r02 = r0 * r0;
        float rTheta = r02 + (1 - r02) * pow((1 - cosTheta), 5.0);
        float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        
        glm::vec3 outDirection = sinTheta * eta > 1.0 || random < rTheta
            ? glm::reflect(in.direction, normal)
            : glm::refract(in.direction, normal, eta);
        out = Ray(outDirection, intersection);
        outColor = WHITE;
        return true;
    }
};

struct Sphere {
    const glm::vec3 center;
    float radius;
    const std::shared_ptr<Material> material;
    
    Sphere(const glm::vec3& center,
           float radius,
           const std::shared_ptr<Material> material) : center(center), radius(radius), material(material) {}
};

struct Camera {
    glm::vec3 position;
    glm::vec2 ccd; // this is the imagined CCD size for the camera (in metric units: cm)
    float focal;
    
    Camera(const glm::vec3& position,
           const glm::vec2& ccd,
           float focal) : position(position), ccd(ccd), focal(focal) {}
    
    // produces the ray from the camera center through a particular normalized pixel coordinate
    Ray generateRay(const glm::vec2& uv) {
        glm::vec2 ccdPosition(uv.x * ccd.x - ccd.x / 2, uv.y * ccd.y - ccd.y / 2);
        glm::vec3 ray = glm::vec3(ccdPosition.x, -ccdPosition.y, -focal) - position;
        glm::vec3 direction = glm::normalize(ray);
        return Ray(direction, position);
    }
};

struct Scene {
    std::vector<Sphere> spheres;
    
    void addSphere(const glm::vec3& center, float radius, const std::shared_ptr<Material> material) {
        spheres.push_back(Sphere(center, radius, material));
    }
};

void writeColor(std::ofstream &out, glm::vec3& color) {
    // Write the translated [0,255] value of each color component.
    float gammaCorrection = 1.0f / FLAGS_samples;
    
    out << static_cast<int>(255 * sqrt(color.x * gammaCorrection)) << ' '
        << static_cast<int>(255 * sqrt(color.y * gammaCorrection)) << ' '
        << static_cast<int>(255 * sqrt(color.z * gammaCorrection)) << '\n';
}

// borrowed from https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
float intersectSphere(const Sphere& sphere, const Ray& ray) {
    glm::vec3 sphereToOrigin = ray.origin - sphere.center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0 * glm::dot(sphereToOrigin, ray.direction);
    float c = glm::dot(sphereToOrigin, sphereToOrigin) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
       return -1.0;
    }
    return (-b - sqrt(discriminant)) / (2.0 * a);
}

glm::vec3 visualizeNormal(const glm::vec3& center, const glm::vec3& spherePoint) {
    glm::vec3 normal = glm::normalize(spherePoint - center);
    return glm::vec3(normal.x + 1, normal.y + 1, normal.z + 1) * 0.5f;
}

/**
 * returns color for the intersection of the ray with the scene. note that THIS is
 * where all the interesting Monte Carlo sampling will be happening!
 *
 */
glm::vec3 findIntersection(const Scene& scene, const Ray& ray, const Camera& camera, int bounce) {
//    std::cout << bounce << std::endl;
    if (bounce >= FLAGS_bounces) {
        return BLACK;
    }
    
    for (const Sphere& sphere : scene.spheres) {
        float intersection = intersectSphere(sphere, ray);
        if (intersection > 0) {
            glm::vec3 intersectionPoint = ray.direction * intersection;
            glm::vec3 normal = glm::normalize(intersectionPoint - sphere.center);
            bool inside = glm::dot(ray.direction, normal) > 0;

            Ray scatteredRay;
            color scatteredColor;
            bool didScatter = sphere.material->scatter(ray,
                                                       intersectionPoint,
                                                       normal,
                                                       inside,
                                                       scatteredRay,
                                                       scatteredColor);
            if (!didScatter) {
                return BLACK;
            }
            return scatteredColor * findIntersection(scene, scatteredRay, camera, bounce + 1);
        }
    }
    
    float t = 0.5 * (ray.direction.y + 1.0);
    return glm::vec3(1.0, 1.0, 1.0) * (1.0f - t) + glm::vec3(0.5, 0.7, 1.0) * t;
}

/**
 * Point on choice of coordinate system
 *
 * One thing to be really clear on to avoid confusion in computer graphics is the choice of coordinate system and
 * if there is any semantic meaning to the units. Here, we choose x right, y up, z out with units being roughly cm
 *
 */
int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    std::ofstream result(FLAGS_filename);
    result << "P3\n" << FLAGS_width << ' ' << FLAGS_height << "\n255\n";
    
    const float imageAspectRatio = FLAGS_width / FLAGS_height;
    const float cameraCCDheight = 2.0; // nice to have ccd have size [-1, 1] by default
    const float cameraCCDwidth = cameraCCDheight * imageAspectRatio;
    
    Camera camera(glm::vec3(0, 0, 0), glm::vec2(cameraCCDwidth, cameraCCDheight), 1.0);
    
    Scene scene;
    scene.addSphere(glm::vec3(0.0, 0.0, -2.0), 0.6, std::make_shared<Lambertian>(PEACH));
    scene.addSphere(glm::vec3(-1.2, 0.0, -2.0),0.5, std::make_shared<Metal>(LIGHT_GRAY, 0.1));
    scene.addSphere(glm::vec3(1.4, 0.0, -2.0), 0.7, std::make_shared<Dielectric>(1.5));
    scene.addSphere(glm::vec3(0.0, -101.0, -2.0), 100.0, std::make_shared<Lambertian>(GREEN));
    
    for (int row = 0; row < FLAGS_height; row++) {
        for (int col = 0; col < FLAGS_width; col++) {
            glm::vec3 color(0, 0, 0);
            for (int sample = 0; sample < FLAGS_samples; sample++) {
                // TODO: here is one spot where sampling can be done more intelligently! just uniform right now
                glm::vec2 uv(
                             ((float)col + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) / FLAGS_width,
                             ((float)row + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) / FLAGS_height);
                Ray ray = camera.generateRay(uv); // implicit origin of rays is the camera position
                color += findIntersection(scene, ray, camera, 0);
            }
            color /= FLAGS_samples;
            writeColor(result, color);
        }
    }
    
    result.close();
    
    return 0;
}
