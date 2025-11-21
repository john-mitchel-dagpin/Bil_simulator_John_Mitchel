#include "World.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

World::World() {
    reset();
}

void World::reset() {
    car_.reset();
    car_.setPosition(0, 0);

    objects_.clear();

    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SpeedBoost, 5.f, 0.f));
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SizeChange, -3.f, -4.f));

    objects_.push_back(std::make_unique<Obstacle>(0.f, 7.f, 2.f, 1.5f));
    objects_.push_back(std::make_unique<Obstacle>(-6.f, 3.f, 1.5f, 1.5f));
}

bool World::intersects(const Car::AABB& a, const GameObject::AABB& b) const {
    return (a.minX <= b.maxX && a.maxX >= b.minX &&
            a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}

void World::update(float dt, const InputState& input) {
    car_.update(dt, input);

    auto carB = car_.bounds();

    for (auto& obj : objects_) {
        if (obj->isActive() && intersects(carB, obj->bounds())) {
            obj->onCarOverlap(car_);
        }
    }
}
