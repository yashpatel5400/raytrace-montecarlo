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

#include <glm/gtc/random.hpp>

glm::vec2 sampleUnitDisc() {
    while (true) {
        glm::vec2 proposal(
                           2.0 * (glm::linearRand(0.0f, 1.0f) - 0.5),
                           2.0 * (glm::linearRand(0.0f, 1.0f) - 0.5)
                           );
        if (glm::length(proposal) <= 1.0) {
            return proposal;
        }
    }
}

struct Camera {
    glm::vec3 position;
    glm::vec2 ccd; // this is the imagined CCD size for the camera (in metric units: cm)
    float focal;
    float aperture; // for an idealized virtual camera with no depth of field, pass in 0

    glm::vec3 up;
    glm::vec3 forward;
    glm::vec3 right;
    
    Camera(const glm::vec3& position,
           const glm::vec2& ccd,
           const glm::vec3& focusPosition,
           float focal,
           float aperture) : position(position), ccd(ccd), focal(focal), aperture(aperture) {
        // axis align the camera by default -- these are modified if we do lookAt
        forward = glm::normalize(focusPosition - position);
        right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::cross(right, forward));
    }
    
    // produces the ray from the camera center through a particular normalized pixel coordinate
    Ray generateRay(const glm::vec2& uv) {
        glm::vec2 ccdPosition(uv.x * ccd.x - ccd.x / 2, uv.y * ccd.y - ccd.y / 2);
        glm::vec2 dofOffset(0, 0);
        if (aperture > 0) {
            dofOffset += sampleUnitDisc() * (aperture / 2.0f);
        }
        glm::vec3 ray = ccdPosition.x * right + -ccdPosition.y * up + focal * forward;
        
        glm::vec3 direction = glm::normalize(ray);
        return Ray(direction, position + glm::vec3(dofOffset, 0));
    }
};

#endif /* camera_hpp */
