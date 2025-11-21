#include "Car.hpp"
#include <cmath>
#include <algorithm>

Car::Car() = default;

void Car::reset() {
    position_ = {0.f, 0.f};
    rotation_ = 0.f;
    speed_ = 0.f;
}

void Car::setPosition(float x, float z) {
    position_ = {x, z};
}

void Car::setRotation(float angle) {
    rotation_ = angle;
}

void Car::update(float dt, const InputState& input) {
    if (input.accelerate) {
        speed_ += acceleration_ * dt;
    }
    if (input.brake) {
        speed_ -= brakeDeceleration_ * dt;
    }

    // friction
    if (!input.accelerate && !input.brake) {
        if (speed_ > 0) speed_ = std::max(0.f, speed_ - friction_ * dt);
        else if (speed_ < 0) speed_ = std::min(0.f, speed_ + friction_ * dt);
    }

    // clamp
    speed_ = std::clamp(speed_, -maxSpeed_ * 0.5f, maxSpeed_);

    // rotation only when moving a bit
    if (std::abs(speed_) > 0.1f) {
        if (input.turnLeft) rotation_ += turnSpeed_ * dt;
        if (input.turnRight) rotation_ -= turnSpeed_ * dt;
    }

    // movement
    position_.x += std::sin(rotation_) * speed_ * dt;
    position_.z += std::cos(rotation_) * speed_ * dt;
}

Car::AABB Car::bounds() const {
    return {
        position_.x - halfWidth_,
        position_.x + halfWidth_,
        position_.z - halfLength_,
        position_.z + halfLength_
    };
}
