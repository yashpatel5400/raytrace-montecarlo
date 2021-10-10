/**
 * @file camera.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef camera_hpp
#define camera_hpp

#include "util.hpp"

struct Camera {
    glm::vec3 position;
    glm::vec2 ccd; // this is the imagined CCD size for the camera (in metric units: cm)
    float focal;
    
    glm::vec3 up;
    glm::vec3 forward;
    glm::vec3 right;
    
    Camera(const glm::vec3& position,
           const glm::vec2& ccd,
           const glm::vec3& focusPosition,
           float focal) : position(position), ccd(ccd), focal(focal) {
        // axis align the camera by default -- these are modified if we do lookAt
        forward = glm::normalize(focusPosition - position);
        right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::cross(right, forward));
    }
    
    // produces the ray from the camera center through a particular normalized pixel coordinate
    Ray generateRay(const glm::vec2& uv) {
        glm::vec2 ccdPosition(uv.x * ccd.x - ccd.x / 2, uv.y * ccd.y - ccd.y / 2);
        glm::vec3 ray = ccdPosition.x * right + -ccdPosition.y * up + focal * forward;
        glm::vec3 direction = glm::normalize(ray);
        return Ray(direction, position);
    }
};

#endif /* camera_hpp */
