#include <catch2/catch_test_macros.hpp>
#include "GameObject.hpp"

class DummyObject : public GameObject {
public:
    bool called = false;

    DummyObject() {
        bounds_ = {0,1,0,1};
    }

    void onCarOverlap(Car&) override {
        called = true;
    }
};

TEST_CASE("GameObject stores bounds") {
    DummyObject obj;

    auto b = obj.bounds();

    REQUIRE(b.minX == 0.f);
    REQUIRE(b.maxX == 1.f);
    REQUIRE(b.minZ == 0.f);
    REQUIRE(b.maxZ == 1.f);
}

TEST_CASE("GameObject is active until deactivated") {
    DummyObject obj;

    REQUIRE(obj.isActive());

    obj.deactivate();

    REQUIRE_FALSE(obj.isActive());
}

TEST_CASE("GameObject calls overlap function") {
    DummyObject obj;
    Car car;

    obj.onCarOverlap(car);

    REQUIRE(obj.called);
}
