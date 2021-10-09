#include <fstream>
#include <iostream>

#include <vector>

#include <gflags/gflags.h>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

DEFINE_string(filename, "", "Desired output file for rendering");
DEFINE_int32(width, 0, "Desired width of rendering");
DEFINE_int32(height, 0, "Desired height of rendering");

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

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    std::ofstream result(FLAGS_filename);
    result << "P3\n" << FLAGS_width << ' ' << FLAGS_height << "\n255\n";
   
    
    
    Scene scene;
    scene.addSphere(FUCHSIA, glm::vec3(0, 3.0, 0), 2.0);
    
    for (int row = 0; row < FLAGS_height; row++) {
        for (int col = 0; col < FLAGS_width; col++) {
            glm::vec3 color(row, col, 0);
            
            writeColor(result, color);
        }
    }
    
    result.close();
    
    return 0;
}
