#ifndef BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#pragma once

#include <vector>
#include <memory>
#include "Car.hpp"
#include "GameObject.hpp"

class Obstacle; // forward declaration

class World {
public:
    World();
    void update(float dt, const InputState& input);
    void reset();

    Car& car() { return car_; }
    const Car& car() const { return car_; }

    const std::vector<std::unique_ptr<GameObject>>& objects() const { return objects_; }

    // Pickup info (for gates & later UI)
    bool allPickupsCollected() const;
    int totalPickups() const;
    int collectedPickups() const;

    // Gate state (for door animations in main.cpp)
    bool gate1IsOpen() const;
    bool gate2IsOpen() const;

private:
    Car car_;
    std::vector<std::unique_ptr<GameObject>> objects_;

    Obstacle* gate1Obstacle_ = nullptr;
    Obstacle* gate2Obstacle_ = nullptr;

    bool intersects(const Car::AABB& a, const GameObject::AABB& b) const;
};

#endif //BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
