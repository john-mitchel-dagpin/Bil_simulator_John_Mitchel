#ifndef BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#pragma once

#include <vector>
#include <memory>

#include "Car.hpp"
#include "GameObject.hpp"

class Obstacle; // forward declaration
class Pickup;   // forward declaration

class World {
public:
    World();
    void update(float dt, const InputState& input);
    void reset();

    Car& car() { return car_; }
    const Car& car() const { return car_; }

    const std::vector<std::unique_ptr<GameObject>>& objects() const { return objects_; }

    // Gate state (for doors in main.cpp)
    bool gate1IsOpen() const; // village gate
    bool gate2IsOpen() const; // castle gate
    bool gate3IsOpen() const; // smelter gate

    // Portal state (for end scene)
    bool portalTriggered() const { return portalTriggered_; }
    Vec2 portalCenter() const { return {portalX_, portalZ_}; }

    // Pickup info (for potential UI)
    int totalPickups() const;
    int collectedPickups() const;
    bool allPickupsCollected() const;

private:
    Car car_;
    std::vector<std::unique_ptr<GameObject>> objects_;

    // gate obstacles (logical blockers)
    Obstacle* gate1Obstacle_ = nullptr;
    Obstacle* gate2Obstacle_ = nullptr;
    Obstacle* gate3Obstacle_ = nullptr;

    // pickups that control each gate (two per gate)
    Pickup* gate1PickupA_ = nullptr;
    Pickup* gate1PickupB_ = nullptr;
    Pickup* gate2PickupA_ = nullptr;
    Pickup* gate2PickupB_ = nullptr;
    Pickup* gate3PickupA_ = nullptr;
    Pickup* gate3PickupB_ = nullptr;

    // portal zone inside mountain
    float portalX_ = 0.f;
    float portalZ_ = 0.f;
    float portalHalfW_ = 4.f;
    float portalHalfL_ = 4.f;
    bool portalTriggered_ = false;

    bool intersects(const Car::AABB& a, const GameObject::AABB& b) const;
};

#endif //BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
