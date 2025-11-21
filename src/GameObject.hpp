
#ifndef BIL_SIMULATOR_JOHN_MITCHEL_GAMEOBJECT_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_GAMEOBJECT_HPP
#pragma once
#include "Car.hpp"

class GameObject {
public:
    virtual ~GameObject() = default;

    struct AABB {
        float minX, maxX;
        float minZ, maxZ;
    };

    AABB bounds() const { return bounds_; }
    bool isActive() const { return active_; }

    // Allow world/objects to deactivate things like doors/obstacles
    void deactivate() { active_ = false; }

    virtual void update(float dt) {}
    virtual void onCarOverlap(Car& car) = 0;

protected:
    AABB bounds_{};
    bool active_ = true;
};


#endif //BIL_SIMULATOR_JOHN_MITCHEL_GAMEOBJECT_HPP