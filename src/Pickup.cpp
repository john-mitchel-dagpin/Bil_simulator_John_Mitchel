#include "Pickup.hpp"
#include <algorithm>

Pickup::Pickup(Type type, float x, float z)
    : type_(type)
{
    float r = 0.8f;
    bounds_ = {x - r, x + r, z - r, z + r};
}

void Pickup::onCarOverlap(Car& car) {
    if (!active_) return;

    switch (type_) {

        case Type::SpeedBoost:
            car.applySpeedBoost();
            break;

        case Type::SizeChange:
            car.applySizeChange();
            break;
    }

    active_ = false;
}
