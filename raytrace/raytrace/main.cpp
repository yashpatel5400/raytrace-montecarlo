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

#define WHITE glm::vec3(1.00, 1.00, 1.00)
#define SILVER glm::vec3(.75, .75, .75)
#define GRAY glm::vec3(.50, .50, .50)
#define BLACK glm::vec3(0, 0, 0)
#define RED glm::vec3(1.00, 0, 0)
#define MAROON glm::vec3(.50, 0, 0)
#define YELLOW glm::vec3(1.00, 1.00, 0)
#define OLIVE glm::vec3(.50, .50, 0)
#define LIME glm::vec3(0, 1.00, 0)
#define GREEN glm::vec3(0, .50, 0)
#define AQUA glm::vec3(0, 1.00, 1.00)
#define TEAL glm::vec3(0, .50, .50)
#define BLUE glm::vec3(0, 0, 1.00)
#define NAVY glm::vec3(0, 0, .50)
#define FUCHSIA glm::vec3(1.00, 0, 1.00)
#define PURPLE glm::vec3(.50, 0, .50)

// global simulation parameters
static const int kSamplesPerPixel = 5;
static const int kMaxBouncesPerSample = 10;

struct Ray {
    glm::vec3 direction;
    glm::vec3 origin;
    
    Ray(const glm::vec3& direction,
           const glm::vec3& origin) : direction(direction), origin(origin) {}
};

struct Sphere {
    const glm::vec3 color;
    const glm::vec3 center;
    float radius;
    
    Sphere(const glm::vec3& color,
           const glm::vec3& center,
           float radius) : color(color), center(center), radius(radius) {}
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
    
    void addSphere(const glm::vec3& color, const glm::vec3& center, float radius) {
        spheres.push_back(Sphere(color, center, radius));
    }
};

void writeColor(std::ofstream &out, glm::vec3& color) {
    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(255 * color.x) << ' '
        << static_cast<int>(255 * color.y) << ' '
        << static_cast<int>(255 * color.z) << '\n';
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

// TODO: here is the other major spot for improving sampling from the BRDF function
glm::vec3 sampleUnitSphere() {
    while (true) {
        glm::vec3 proposal(
                           static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                           static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                           static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
                           );
        if (proposal.length() <= 1.0) {
            return glm::normalize(proposal);
        }
    }
}

/**
 * returns color for the intersection of the ray with the scene. note that THIS is
 * where all the interesting Monte Carlo sampling will be happening!
 *
 */
glm::vec3 findIntersection(const Scene& scene, const Ray& ray, const Camera& camera, int bounce) {
    if (bounce > kMaxBouncesPerSample) {
        return BLACK;
    }
    
    for (const Sphere& sphere : scene.spheres) {
        float intersection = intersectSphere(sphere, ray);
        if (intersection > 0) {
//            return visualizeNormal(sphere.center, ray.origin + ray.direction * intersection);
            glm::vec3 newOrigin = ray.direction * intersection;
            glm::vec3 newDirection = sampleUnitSphere();
            return 0.5f * findIntersection(scene, Ray(newDirection, newOrigin), camera, bounce + 1);
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
    scene.addSphere(FUCHSIA, glm::vec3(0.0, 0.0, -2.0), 1.0);
    scene.addSphere(GREEN, glm::vec3(0.0, -10.0, -2.5), 10.0);
    
    for (int row = 0; row < FLAGS_height; row++) {
        for (int col = 0; col < FLAGS_width; col++) {
            glm::vec3 color(0, 0, 0);
            for (int sample = 0; sample < kSamplesPerPixel; sample++) {
                // TODO: here is one spot where sampling can be done more intelligently! just uniform right now
                glm::vec2 uv(
                             ((float)col + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) / FLAGS_width,
                             ((float)row + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) / FLAGS_height);
                Ray ray = camera.generateRay(uv); // implicit origin of rays is the camera position
                color += findIntersection(scene, ray, camera, 0);
            }
            color /= kSamplesPerPixel;
            writeColor(result, color);
        }
    }
    
    result.close();
    
    return 0;
}
