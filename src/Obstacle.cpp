#include "Obstacle.hpp"
#include <cmath>
#include <algorithm> // REQUIRED for std::min/std::max

Obstacle::Obstacle(float x, float z, float halfW, float halfL) {
    bounds_ = {x - halfW, x + halfW, z - halfL, z + halfL};
}

void Obstacle::onCarOverlap(Car& car) {
    auto cb = car.bounds();
    auto ob = bounds_;

    // Compute overlap on each axis
    float overlapX1 = ob.maxX - cb.minX;
    float overlapX2 = cb.maxX - ob.minX;
    float overlapX = std::min(overlapX1, overlapX2);

    float overlapZ1 = ob.maxZ - cb.minZ;
    float overlapZ2 = cb.maxZ - ob.minZ;
    float overlapZ = std::min(overlapZ1, overlapZ2);

    // Move along the smallest overlap axis
    float px = car.position().x;
    float pz = car.position().z;

    if (overlapX < overlapZ) {
        // Resolve X axis collision
        if (overlapX1 < overlapX2) {
            px = ob.maxX + car.getHalfWidth();   // push right
        } else {
            px = ob.minX - car.getHalfWidth();   // push left
        }
        car.setPosition(px, pz);
    } else {
        // Resolve Z axis collision
        if (overlapZ1 < overlapZ2) {
            pz = ob.maxZ + car.getHalfLength();  // push forward
        } else {
            pz = ob.minZ - car.getHalfLength();  // push backward
        }
        car.setPosition(px, pz);
    }

    // Optional: slow car when hitting a wall
    // car.stopSpeed();
}
