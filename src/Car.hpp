//
// Created by johnm on 21.11.2025.
//

#ifndef BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP
#pragma once
#pragma once
#include "InputState.hpp"

struct Vec2 {
    float x{};
    float z{};
};

class Car {
public:
    Car();

    void update(float dt, const InputState& input);
    void reset();

    void setPosition(float x, float z);
    void setRotation(float angle);

    Vec2 position() const { return position_; }
    float rotation() const { return rotation_; }
    float speed() const { return speed_; }

    struct AABB {
        float minX, maxX;
        float minZ, maxZ;
    };

    AABB bounds() const;

private:
    Vec2 position_{};
    float rotation_ = 0.f;
    float speed_ = 0.f;

    float maxSpeed_ = 20.f;
    float acceleration_ = 15.f;
    float brakeDeceleration_ = 25.f;
    float friction_ = 5.f;
    float turnSpeed_ = 2.5f;

    float halfWidth_ = 1.f;
    float halfLength_ = 2.f;
};

#endif //BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP