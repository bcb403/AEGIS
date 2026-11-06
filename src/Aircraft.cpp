#include "Aircraft.h"

Aircraft::Aircraft() = default;

const AircraftState& Aircraft::getState() const {
    return state;
}

void Aircraft::update(double deltaTime) {
    (void)deltaTime;
}