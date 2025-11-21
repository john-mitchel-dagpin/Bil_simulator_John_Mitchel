
#ifndef BIL_SIMULATOR_JOHN_MITCHEL_PICKUP_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_PICKUP_HPP
#pragma once
#include "GameObject.hpp"

class Pickup : public GameObject {
public:
    enum class Type { SpeedBoost, SizeChange };

    Pickup(Type type, float x, float z);
    void onCarOverlap(Car& car) override;

private:
    Type type_;
};


#endif //BIL_SIMULATOR_JOHN_MITCHEL_PICKUP_HPP