#include "Car.hpp"
#include <algorithm>

void Car::update(float dt, bool forward, bool backward, bool left, bool right) {

    // throttle and brake
    float throttle = forward ? 1.f : 0.f;
    float brake    = backward ? 1.f : 0.f;

    // longitudinal speed
    speed += (throttle * accel - brake * brakeAccel - drag * speed) * dt;

    // clamp speed (allow limited reverse)
    if (speed >  maxSpeed)          speed =  maxSpeed;
    if (speed < -maxSpeed * 0.35f)  speed = -maxSpeed * 0.35f;

    // steering input
    float steerInput = 0.f;
    if (left)  steerInput += 1.f;
    if (right) steerInput -= 1.f;

    // target steering
    float target = steerInput * maxSteer;

    // rate-limit change of steering
    float diff = target - steerAngle;
    float step = steerRate * dt;
    if (diff >  step) diff =  step;
    if (diff < -step) diff = -step;
    steerAngle += diff;

    // auto-center when no input
    if (steerInput == 0.f) {
        if (std::fabs(steerAngle) <= step) steerAngle = 0.f;
        else steerAngle += (steerAngle > 0 ? -step : step);
    }

    // bicycle model
    heading += (speed / wheelbase) * std::tan(steerAngle) * dt;

    // integrate position
    position.x += std::cos(heading) * speed * dt;
    position.y += std::sin(heading) * speed * dt;
}

bool Car::checkCollision(const Vec2 &objPos, float objRadius, float carRadius) const {
    float r = objRadius + carRadius;
    return dist(position, objPos) <= r;
}

void Car::reset() {
    position   = Vec2{0.f, 0.f};
    heading    = 0.f;
    speed      = 0.f;
    steerAngle = 0.f;
}
