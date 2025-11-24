#include "World.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

World::World() {
    reset();
}

void World::reset() {

    car_.reset();
    car_.setPosition(0.f, 0.f); // start in center

    objects_.clear();

    // reset gate pointers
    gate1Obstacle_ = nullptr;
    gate2Obstacle_ = nullptr;
    gate3Obstacle_ = nullptr;

    gate1PickupA_ = gate1PickupB_ = nullptr;
    gate2PickupA_ = gate2PickupB_ = nullptr;
    gate3PickupA_ = gate3PickupB_ = nullptr;

    portalTriggered_ = false;

    // --------------------------------------------------
    //  PICKUPS (2 per gate)
    // --------------------------------------------------

    auto addPickup = [&](Pickup::Type t, float x, float z, Pickup*& out) {
        auto p = std::make_unique<Pickup>(t, x, z);
        out = p.get();
        objects_.push_back(std::move(p));
    };

    // Gate 1 pickups (village)
    addPickup(Pickup::Type::SpeedBoost, -100.f, -100.f, gate1PickupA_);
    addPickup(Pickup::Type::SizeChange, -100.f,  -80.f, gate1PickupB_);

    // Gate 2 pickups (castle)
    addPickup(Pickup::Type::SpeedBoost,   0.f, 90.f, gate2PickupA_);
    addPickup(Pickup::Type::SizeChange, -10.f, 90.f, gate2PickupB_);

    // Gate 3 pickups (smelter)
    addPickup(Pickup::Type::SpeedBoost,  90.f, -100.f, gate3PickupA_);
    addPickup(Pickup::Type::SizeChange,  90.f, -110.f, gate3PickupB_);

    // --------------------------------------------------
    //  400x400 WORLD BORDER (ONLY COLLISION WALLS NOW)
    // --------------------------------------------------
    const float B = 200.f; // near edge of 400×400 plane
    const float T = 1.f;   // thickness

    // north/south
    objects_.push_back(std::make_unique<Obstacle>(0.f,  B, 200.f, T));
    objects_.push_back(std::make_unique<Obstacle>(0.f, -B, 200.f, T));

    // west/east
    objects_.push_back(std::make_unique<Obstacle>(-B, 0.f, T, 200.f));
    objects_.push_back(std::make_unique<Obstacle>( B, 0.f, T, 200.f));

     // Village fence (doesn't block gate opening )
     objects_.push_back(std::make_unique<Obstacle>(-120.f, -158.f, 0.5f, 25.5f)); // south fence
     objects_.push_back(std::make_unique<Obstacle>(-120.f, -92.f, 0.5f, 25.5f));  // north fence
     objects_.push_back(std::make_unique<Obstacle>(-160.f, -185.f, 40.f, 0.5f));  // west fence
     objects_.push_back(std::make_unique<Obstacle>(-160.f,  -65.f, 40.f, 0.5f));  // east fence except gate gap

     // Castle fence (gate at 0,100)
     objects_.push_back(std::make_unique<Obstacle>(27.f, 100.f, 22.f, 1.f)); // north wall
     objects_.push_back(std::make_unique<Obstacle>(-20.f, 100.f, 15.f, 1.f));  // south wall
     objects_.push_back(std::make_unique<Obstacle>(50.f, 150.f, 1.f, 50.5f)); // west wall
     objects_.push_back(std::make_unique<Obstacle>(-35.f, 150.f, 1.f, 50.5f));  // east wall
    //
    // // Smelter fence (gate at 110,-120)
    // objects_.push_back(std::make_unique<Obstacle>(80.f, -150.f, 120.f, 3.f)); // south
    // objects_.push_back(std::make_unique<Obstacle>(80.f, -70.f, 120.f, 3.f));  // north
    // objects_.push_back(std::make_unique<Obstacle>(40.f, -110.f, 3.f, 80.f));  // west
    // objects_.push_back(std::make_unique<Obstacle>(160.f,-110.f, 3.f, 80.f));  // east






    // --------------------------------------------------
    //  GATES (ONLY THESE BLOCK INSIDE)
    // --------------------------------------------------

    // Gate 1 – Village
    {
        auto g = std::make_unique<Obstacle>(-120.f, -125.f, 3.f, 8.f);
        gate1Obstacle_ = g.get();
        objects_.push_back(std::move(g));
    }

    // Gate 2 – Castle
    {
        auto g = std::make_unique<Obstacle>(0.f, 100.f, 8.f, 3.f);
        gate2Obstacle_ = g.get();
        objects_.push_back(std::move(g));
    }

    // Gate 3 – Smelter
    {
        auto g = std::make_unique<Obstacle>(110.f, -120.f, 3.f, 8.f);
        gate3Obstacle_ = g.get();
        objects_.push_back(std::move(g));
    }

    // --------------------------------------------------
    //  PORTAL (NO COLLISION BLOCK INSIDE)
    // --------------------------------------------------
    portalX_ = -150.f;
    portalZ_ = 120.f;
    portalHalfW_ = 6.f;
    portalHalfL_ = 6.f;
}

bool World::intersects(const Car::AABB& a, const GameObject::AABB& b) const {
    return (a.minX <= b.maxX && a.maxX >= b.minX &&
            a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}

void World::update(float dt, const InputState& input) {

    if (!portalTriggered_) {
        car_.update(dt, input);
    }

    auto carB = car_.bounds();

    // collisions
    for (auto& obj : objects_) {
        if (obj->isActive() && intersects(carB, obj->bounds())) {
            obj->onCarOverlap(car_);
        }
    }

    // Gate logic
    auto collected = [&](Pickup* A, Pickup* B) {
        return A && B && !A->isActive() && !B->isActive();
    };

    if (gate1Obstacle_ && gate1Obstacle_->isActive() &&
        collected(gate1PickupA_, gate1PickupB_)) {
        gate1Obstacle_->deactivate();
    }

    if (gate2Obstacle_ && gate2Obstacle_->isActive() &&
        collected(gate2PickupA_, gate2PickupB_)) {
        gate2Obstacle_->deactivate();
    }

    if (gate3Obstacle_ && gate3Obstacle_->isActive() &&
        collected(gate3PickupA_, gate3PickupB_)) {
        gate3Obstacle_->deactivate();
    }

    // Portal
    if (!portalTriggered_) {
        GameObject::AABB portalBox{
            portalX_ - portalHalfW_, portalX_ + portalHalfW_,
            portalZ_ - portalHalfL_, portalZ_ + portalHalfL_
        };

        if (intersects(carB, portalBox)) {
            portalTriggered_ = true;
        }
    }
}

bool World::gate1IsOpen() const { return !gate1Obstacle_ || !gate1Obstacle_->isActive(); }
bool World::gate2IsOpen() const { return !gate2Obstacle_ || !gate2Obstacle_->isActive(); }
bool World::gate3IsOpen() const { return !gate3Obstacle_ || !gate3Obstacle_->isActive(); }

int World::totalPickups() const {
    int c = 0;
    for (auto& o : objects_) if (dynamic_cast<Pickup*>(o.get())) c++;
    return c;
}

int World::collectedPickups() const {
    int c = 0;
    for (auto& o : objects_) {
        if (auto p = dynamic_cast<Pickup*>(o.get())) {
            if (!p->isActive()) c++;
        }
    }
    return c;
}

bool World::allPickupsCollected() const {
    return totalPickups() == collectedPickups() && totalPickups() > 0;
}
