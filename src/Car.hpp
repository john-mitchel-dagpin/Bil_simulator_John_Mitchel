#ifndef BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP

#pragma once
#include "physics.hpp"
#include <cmath>

struct Car {

    // State
    Vec2  position{0.f, 0.f};
    float heading   = 0.f;
    float speed     = 0.f;

    // Dynamics tuning
    float accel      = 4.0f;
    float brakeAccel = 6.0f;
    float drag       = 0.8f;
    float maxSpeed   = 8.0f;

    // Steering
    float steerAngle = 0.0f;
    float maxSteer   = 30.0f * 3.14159265f / 180.f;   // 30 degrees
    float steerRate  = 180.0f * 3.14159265f / 180.f;  // 180 deg/sec
    float wheelbase  = 1.2f;                          // meters

    void update(float dt, bool forward, bool backward, bool left, bool right);
    bool checkCollision(const Vec2& objPos, float objRadius, float carRadius) const;
    void reset();
};

#endif
