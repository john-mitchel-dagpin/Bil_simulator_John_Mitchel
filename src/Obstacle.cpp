#include "Obstacle.hpp"
#include <algorithm>

Obstacle::Obstacle(float x, float z, float halfW, float halfL) {
    bounds_ = {x - halfW, x + halfW, z - halfL, z + halfL};
}

void Obstacle::onCarOverlap(Car& car) {
    // simple collision: stop the car
    car.setPosition(
            std::clamp(car.position().x, bounds_.minX, bounds_.maxX),
            std::clamp(car.position().z, bounds_.minZ, bounds_.maxZ)
    );
}
