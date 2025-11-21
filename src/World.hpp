

#ifndef BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP
#pragma once
#include <vector>
#include <memory>
#include "Car.hpp"
#include "GameObject.hpp"

class World {
public:
    World();
    void update(float dt, const InputState& input);
    void reset();

    Car& car() { return car_; }
    const Car& car() const { return car_; }
    const std::vector<std::unique_ptr<GameObject>>& objects() const { return objects_; }

private:
    Car car_;
    std::vector<std::unique_ptr<GameObject>> objects_;

    bool intersects(const Car::AABB& a, const GameObject::AABB& b) const;
};


#endif //BIL_SIMULATOR_JOHN_MITCHEL_WORLD_HPP