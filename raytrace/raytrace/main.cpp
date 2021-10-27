/**
 * @file main.cpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#include "camera.hpp"
#include "material.hpp"
#include "scene.hpp"

#include <fstream>
#include <iostream>

#include <gflags/gflags.h>
#include <glm/gtc/random.hpp>

DEFINE_string(filename, "", "Output file for rendering");
DEFINE_int32(width, 0, "Width of rendering");
DEFINE_int32(height, 0, "Height of rendering");
DEFINE_int32(samples, 5, "Number of samples per pixel");
DEFINE_int32(bounces, 1, "Depth of bounces");

void writeColor(std::ofstream &out, Color& color) {
    out << static_cast<int>(255 * sqrt(color.x / FLAGS_samples)) << ' '
        << static_cast<int>(255 * sqrt(color.y / FLAGS_samples)) << ' '
        << static_cast<int>(255 * sqrt(color.z / FLAGS_samples)) << '\n';
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
    
    const float theta = M_PI / 4;
    float h = tan(theta/2);
    float cameraCCDheight = 2.0 * h;
    float cameraCCDwidth = imageAspectRatio * cameraCCDheight;
    const float focal = 1.0;
    const float aperture = 0.0;
    
    const glm::vec3 lookFrom = glm::vec3(0, 0, 0);
    const glm::vec3 lookAt = glm::vec3(0, 0, -1);
    
    Camera camera(lookFrom, glm::vec2(cameraCCDwidth, cameraCCDheight), lookAt, focal, aperture);
    Scene scene = generateCornellBoxScene();
    
    for (int row = 0; row < FLAGS_height; row++) {
        for (int col = 0; col < FLAGS_width; col++) {
            Color color(0, 0, 0);
            for (int sample = 0; sample < FLAGS_samples; sample++) {
                // TODO: here is one spot where sampling can be done more intelligently! just uniform right now
                glm::vec2 uv(
                             ((float)col + glm::linearRand(0.0f, 1.0f)) / FLAGS_width,
                             ((float)row + glm::linearRand(0.0f, 1.0f)) / FLAGS_height);
                Ray ray = camera.generateRay(uv); // implicit origin of rays is the camera position
                color += castRay(scene, ray, FLAGS_bounces);
            }
            writeColor(result, color);
        }
    }
    
    result.close();
    
    return 0;
}
