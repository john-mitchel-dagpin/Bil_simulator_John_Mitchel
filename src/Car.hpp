#ifndef BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP

#pragma once
#include "InputState.hpp"

struct Vec2 {
    float x{};
    float z{};
};

class Car {
public:
    Car();

    void applySpeedBoost();
    void applySizeChange();

    void update(float dt, const InputState& input);
    void reset();

    void setPosition(float x, float z);
    void setRotation(float angle);
    void setSpeed(float s) { speed_ = s; }


    Vec2 position() const { return position_; }
    float rotation() const { return rotation_; }
    float speed() const { return speed_; }

    struct AABB {
        float minX, maxX;
        float minZ, maxZ;
    };

    AABB bounds() const;

    // ---- PUBLIC GETTERS (required for obstacle + main.cpp) ----
    float getHalfWidth() const { return halfWidth_; }
    float getHalfLength() const { return halfLength_; }
    float getVisualScale() const { return visualScale_; }

private:
    Vec2 position_{};
    float rotation_ = 0.f;
    float speed_ = 0.f;

    float boostTimer_ = 0.f;
    float sizeTimer_ = 0.f;
    bool enlarged_ = false;

    float maxSpeed_ = 30.f;
    float acceleration_ = 15.f;
    float brakeDeceleration_ = 25.f;
    float friction_ = 5.f;
    float turnSpeed_ = 2.5f;

    float halfWidth_ = 1.f;
    float halfLength_ = 2.f;

    // ---- VISUAL SCALE FIELD (used to scale the mesh) ----
    float visualScale_ = 1.f;
};

#endif // BIL_SIMULATOR_JOHN_MITCHEL_CAR_HPP
