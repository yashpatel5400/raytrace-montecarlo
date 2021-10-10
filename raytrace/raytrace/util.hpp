/**
 * @file util.hpp
 *
 * @author Yash Patel
 * Contact: yppatel@umich.edu
 *
 */

#ifndef util_h
#define util_h

#include <vector>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/gtx/string_cast.hpp>

using Color = glm::vec3;

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

#endif /* util_h */
