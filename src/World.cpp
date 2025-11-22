#include "World.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

World::World() {
    reset();
}

void World::reset() {
    car_.reset();
    car_.setPosition(0.f, -30.f); // start further south on the big map

    objects_.clear();
    gate1Obstacle_ = nullptr;
    gate2Obstacle_ = nullptr;

    // =====================================================
    //                 PICKUPS (3 ZONES)
    // =====================================================

    // Zone 1 (starting meadow, around z -30..15)
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SpeedBoost, 0.f, -20.f));
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SizeChange, 4.f, -5.f));

    // Zone 2 (forest / mid area, z ~ 15..70)
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SpeedBoost, -4.f, 35.f));
    objects_.push_back(std::make_unique<Pickup>(Pickup::Type::SizeChange, 4.f, 55.f));

    // You can add more later if you want more collectibles.


    // =====================================================
    //                 NATURAL BOUNDARIES / CLIFFS
    // =====================================================



    // Back cliff behind start (south boundary)
    objects_.push_back(std::make_unique<Obstacle>(0.f, -45.f, 13.f, 1.0f));

    // Far north cliff (end of world)
    objects_.push_back(std::make_unique<Obstacle>(0.f, 150.f, 13.f, 1.0f));


    // =====================================================
    //                 ROCKS / BOULDERS (obstacles)
    // =====================================================

    // Zone 1 boulders
    objects_.push_back(std::make_unique<Obstacle>(-3.f, -15.f, 1.f, 1.f));
    objects_.push_back(std::make_unique<Obstacle>(2.f, -8.f, 1.f, 1.f));

    // Zone 2 boulders
    objects_.push_back(std::make_unique<Obstacle>(-2.f, 30.f, 1.f, 1.f));
    objects_.push_back(std::make_unique<Obstacle>(3.f, 42.f, 1.f, 1.f));
    objects_.push_back(std::make_unique<Obstacle>(0.f, 60.f, 1.5f, 1.5f));

    // Zone 3 boulders (after gate2, z ~ 80+)
    objects_.push_back(std::make_unique<Obstacle>(-3.f, 90.f, 1.5f, 1.5f));
    objects_.push_back(std::make_unique<Obstacle>(4.f, 105.f, 1.f, 1.f));


    // =====================================================
    //                 GATES BETWEEN ZONES
    // =====================================================

    // Gate 1 between Zone 1 and 2, around z ~ 15
    {
        auto gate = std::make_unique<Obstacle>(0.f, 35.f, 3.f, 0.7f);
        gate1Obstacle_ = gate.get();
        objects_.push_back(std::move(gate));
    }

    // Gate 2 between Zone 2 and 3, around z ~ 75
    {
        auto gate = std::make_unique<Obstacle>(0.f, 75.f, 3.f, 0.7f);
        gate2Obstacle_ = gate.get();
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

    // ---- Gate logic based on pickup counts ----
    int total = 0;
    int collected = 0;

    for (const auto& obj : objects_) {
        if (auto pickup = dynamic_cast<Pickup*>(obj.get())) {
            ++total;
            if (!pickup->isActive()) {
                ++collected;
            }
        }
    }

    // Gate 1 opens when at least 2 pickups collected
    if (gate1Obstacle_ && gate1Obstacle_->isActive() && collected >= 2) {
        gate1Obstacle_->deactivate();
    }

    // Gate 2 opens when ALL pickups collected
    if (gate2Obstacle_ && gate2Obstacle_->isActive() && total > 0 && collected == total) {
        gate2Obstacle_->deactivate();
    }
}

bool World::allPickupsCollected() const {
    return collectedPickups() == totalPickups() && totalPickups() > 0;
}

int World::totalPickups() const {
    int total = 0;
    for (const auto& obj : objects_) {
        if (dynamic_cast<Pickup*>(obj.get())) {
            ++total;
        }
    }
    return total;
}

int World::collectedPickups() const {
    int collected = 0;
    for (const auto& obj : objects_) {
        if (auto pickup = dynamic_cast<Pickup*>(obj.get())) {
            if (!pickup->isActive()) {
                ++collected;
            }
        }
    }
    return collected;
}

bool World::gate1IsOpen() const {
    return !gate1Obstacle_ || !gate1Obstacle_->isActive();
}

bool World::gate2IsOpen() const {
    return !gate2Obstacle_ || !gate2Obstacle_->isActive();
}
