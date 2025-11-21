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
    gateObstacle_ = nullptr;

    // Pickups
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SpeedBoost, 5.f, 0.f));
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SizeChange, -3.f, -4.f));

    // Obstacles
    objects_.push_back(std::make_unique<Obstacle>(0.f, 7.f, 2.f, 1.5f));
    objects_.push_back(std::make_unique<Obstacle>(-6.f, 3.f, 1.5f, 1.5f));

    // Gate obstacle that blocks the path near the door
    {
        auto gate = std::make_unique<Obstacle>(0.f, 12.f, 2.f, 0.5f);
        gateObstacle_ = gate.get();
        objects_.push_back(std::move(gate));
    }
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

    // Open the gate obstacle once all pickups are collected
    if (gateObstacle_ && gateObstacle_->isActive()) {
        bool allCollected = true;

        for (const auto& obj : objects_) {
            if (auto pickup = dynamic_cast<Pickup*>(obj.get())) {
                if (pickup->isActive()) {
                    allCollected = false;
                    break;
                }
            }
        }

        if (allCollected) {
            gateObstacle_->deactivate();
        }
    }
}

bool World::allPickupsCollected() const {
    for (const auto& obj : objects_) {
        if (auto pickup = dynamic_cast<Pickup*>(obj.get())) {
            if (pickup->isActive()) {
                return false;
            }
        }
    }
    return true;
}
