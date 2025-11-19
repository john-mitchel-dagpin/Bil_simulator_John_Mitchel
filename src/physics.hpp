

#ifndef BIL_SIMULATOR_JOHN_MITCHEL_PHYSICS_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_PHYSICS_HPP

#pragma once
#include <cmath>

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
    Vec2() = default;
    Vec2(float xx, float yy): x(xx), y(yy) {}
};

inline float dist(const Vec2& a, const Vec2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

#endif //BIL_SIMULATOR_JOHN_MITCHEL_PHYSICS_HPP