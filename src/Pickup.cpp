#include "Pickup.hpp"

Pickup::Pickup(Type type, float x, float z)
    : type_(type)
{
    float r = 0.8f;
    bounds_ = {x - r, x + r, z - r, z + r};
}

void Pickup::onCarOverlap(Car& car) {
    if (!active_) return;

    if (type_ == Type::SpeedBoost) {
        // naive boost
        car.setRotation(car.rotation());
    }

    active_ = false;
}

