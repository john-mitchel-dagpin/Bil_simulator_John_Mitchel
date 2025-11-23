#include "World.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

World::World() {
    reset();
}

void World::reset() {

    car_.reset();
    car_.setPosition(0.f, 0.f); // start in the central area

    objects_.clear();

    gate1Obstacle_ = nullptr;
    gate2Obstacle_ = nullptr;
    gate3Obstacle_ = nullptr;

    gate1PickupA_ = gate1PickupB_ = nullptr;
    gate2PickupA_ = gate2PickupB_ = nullptr;
    gate3PickupA_ = gate3PickupB_ = nullptr;

    portalTriggered_ = false;

    // --------------------------------------------------
    //  PICKUPS – 2 per gate
    // --------------------------------------------------
    auto makePickup = [this](Pickup::Type type, float x, float z, Pickup*& outPtr) {
        auto p = std::make_unique<Pickup>(type, x, z);
        outPtr = p.get();
        objects_.push_back(std::move(p));
    };

    // Gate 1 (village gate on the left)
    makePickup(Pickup::Type::SpeedBoost, -30.f, -10.f, gate1PickupA_);
    makePickup(Pickup::Type::SizeChange, -30.f,  10.f, gate1PickupB_);

    // Gate 2 (castle gate at the top)
    makePickup(Pickup::Type::SpeedBoost,   0.f, 40.f, gate2PickupA_);
    makePickup(Pickup::Type::SizeChange, -10.f, 55.f, gate2PickupB_);

    // Gate 3 (smelter gate on the bottom-right)
    makePickup(Pickup::Type::SpeedBoost,  35.f, -30.f, gate3PickupA_);
    makePickup(Pickup::Type::SizeChange,  45.f, -40.f, gate3PickupB_);

    // --------------------------------------------------
    //  WORLD BORDER (400x400 plane, so +/-200)
    //  thin walls at the edges
    // --------------------------------------------------
    const float BORDER = 195.f;
    const float THICK  = 5.f;

    // north / south
    objects_.push_back(std::make_unique<Obstacle>(0.f,  BORDER, 200.f, THICK));
    objects_.push_back(std::make_unique<Obstacle>(0.f, -BORDER, 200.f, THICK));

    // west / east
    objects_.push_back(std::make_unique<Obstacle>(-BORDER, 0.f, THICK, 200.f));
    objects_.push_back(std::make_unique<Obstacle>( BORDER, 0.f, THICK, 200.f));

    // --------------------------------------------------
    //  AREA WALLS AROUND BUILDINGS (simple rectangles)
    //  these roughly match where your OBJ models go
    // --------------------------------------------------

    // --- Villages on the left ---
    // lower village area
    objects_.push_back(std::make_unique<Obstacle>(-120.f, -60.f, 30.f, 20.f));
    // upper village area
    objects_.push_back(std::make_unique<Obstacle>(-120.f,  10.f, 30.f, 20.f));

    // --- Mountain area (top-left) ---
    // keep a gap on the inside for the portal path
    objects_.push_back(std::make_unique<Obstacle>(-120.f, 110.f, 30.f, 20.f)); // left part
    objects_.push_back(std::make_unique<Obstacle>( -60.f, 140.f, 20.f, 20.f)); // top part

    // --- Castle area (top-center) ---
    objects_.push_back(std::make_unique<Obstacle>(  40.f, 120.f, 30.f, 20.f)); // main castle body

    // --- Archer tower area (right side) ---
    objects_.push_back(std::make_unique<Obstacle>( 130.f, 50.f, 20.f, 20.f));

    // --- Smelter area (bottom-right) ---
    objects_.push_back(std::make_unique<Obstacle>( 100.f, -100.f, 25.f, 25.f));

    // --------------------------------------------------
    //  GATES (one obstacle per gate)
    // --------------------------------------------------

    // Gate 1: between central area and villages on the left
    {
        auto gate = std::make_unique<Obstacle>(-60.f, 0.f, 3.f, 8.f);
        gate1Obstacle_ = gate.get();
        objects_.push_back(std::move(gate));
    }

    // Gate 2: between central area and castle at the top
    {
        auto gate = std::make_unique<Obstacle>(0.f, 70.f, 6.f, 3.f);
        gate2Obstacle_ = gate.get();
        objects_.push_back(std::move(gate));
    }

    // Gate 3: between central area and smelter at the bottom-right
    {
        auto gate = std::make_unique<Obstacle>(50.f, -50.f, 4.f, 4.f);
        gate3Obstacle_ = gate.get();
        objects_.push_back(std::move(gate));
    }

    // --------------------------------------------------
    //  PORTAL ZONE (inside the mountain area)
    // --------------------------------------------------
    portalX_      = -80.f;
    portalZ_      = 120.f;
    portalHalfW_  = 6.f;
    portalHalfL_  = 6.f;
    portalTriggered_ = false;
}

bool World::intersects(const Car::AABB& a, const GameObject::AABB& b) const {
    return (a.minX <= b.maxX && a.maxX >= b.minX &&
            a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}

void World::update(float dt, const InputState& input) {

    if (!portalTriggered_) {
        // only move car & physics while game is "running"
        car_.update(dt, input);
    }

    auto carB = car_.bounds();

    // normal collisions
    for (auto& obj : objects_) {
        if (obj->isActive() && intersects(carB, obj->bounds())) {
            obj->onCarOverlap(car_);
        }
    }

    // ----- Gate logic: open when BOTH pickups for that gate are collected -----

    auto pickupsCollected = [](Pickup* a, Pickup* b) {
        return a && b && !a->isActive() && !b->isActive();
    };

    if (gate1Obstacle_ && gate1Obstacle_->isActive() &&
        pickupsCollected(gate1PickupA_, gate1PickupB_)) {
        gate1Obstacle_->deactivate();
    }

    if (gate2Obstacle_ && gate2Obstacle_->isActive() &&
        pickupsCollected(gate2PickupA_, gate2PickupB_)) {
        gate2Obstacle_->deactivate();
    }

    if (gate3Obstacle_ && gate3Obstacle_->isActive() &&
        pickupsCollected(gate3PickupA_, gate3PickupB_)) {
        gate3Obstacle_->deactivate();
    }

    // ----- Portal detection -----
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

// ---------------- gate getters -----------------

bool World::gate1IsOpen() const {
    return !gate1Obstacle_ || !gate1Obstacle_->isActive();
}

bool World::gate2IsOpen() const {
    return !gate2Obstacle_ || !gate2Obstacle_->isActive();
}

bool World::gate3IsOpen() const {
    return !gate3Obstacle_ || !gate3Obstacle_->isActive();
}

// ---------------- pickup helpers (for possible UI) -----------------

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
            if (!pickup->isActive()) ++collected;
        }
    }
    return collected;
}

bool World::allPickupsCollected() const {
    const int total = totalPickups();
    return total > 0 && collectedPickups() == total;
}
