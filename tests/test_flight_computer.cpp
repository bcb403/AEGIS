
#include "Aircraft.h"
#include "Altimeter.h"
#include "FlightComputer.h"

#include <cmath>
#include <iostream>

bool testSampleCount() {
    Aircraft aircraft;
    Altimeter altimeter;
    FlightComputer computer;

    constexpr int frequency = 50;
    constexpr int duration = 60;
    constexpr double dt = 1.0 / frequency;

    for (int i = 0; i < frequency * duration; ++i) {
        computer.sample(aircraft, altimeter, i * dt);
        aircraft.update(dt);
    }

    return computer.getSampleCount() == 3000;
}

bool testTimestamps() {
    Aircraft aircraft;
    Altimeter altimeter;
    FlightComputer computer;

    constexpr double dt = 0.02;

    for (int i = 0; i < 50; ++i) {
        computer.sample(aircraft, altimeter, i * dt);
    }

    const auto& samples = computer.getSamples();

    if (samples.size() != 50) {
        return false;
    }

    constexpr double tolerance = 1e-9;

    for (int i = 0; i < 50; ++i) {
        double expected = i * dt;

        if (std::abs(samples[i].timestamp_sec - expected)
            > tolerance) {
            return false;
        }
    }

    return true;
}

bool testStateIntegrity() {
    Aircraft aircraft;
    Altimeter altimeter;
    FlightComputer computer;

    computer.sample(aircraft, altimeter, 0.0);

    const auto& samples = computer.getSamples();

    if (samples.size() != 1) {
        return false;
    }

    const auto& state = samples[0].state;

    return state.altitude_ft == 10000.0 &&
           state.airspeed_kts == 180.0 &&
           state.heading_deg == 90.0 &&
           state.pitch_deg == 0.0 &&
           state.roll_deg == 0.0 &&
           state.vertical_speed_fpm == 0.0;
}

bool testAltimeterMeasurement() {
    Aircraft aircraft;
    Altimeter altimeter(0.0);
    FlightComputer computer;

    computer.sample(aircraft, altimeter, 0.0);

    const auto& samples = computer.getSamples();

    if (samples.size() != 1) {
        return false;
    }

    return samples[0].measured_altitude_ft ==
           samples[0].state.altitude_ft;
}

int main() {
    int passed = 0;

    if (testSampleCount()) {
        std::cout << "[PASS] Sample count\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Sample count\n";
    }

    if (testTimestamps()) {
        std::cout << "[PASS] Timestamp accuracy\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Timestamp accuracy\n";
    }

    if (testStateIntegrity()) {
        std::cout << "[PASS] State integrity\n";
        ++passed;
    } else {
        std::cout << "[FAIL] State integrity\n";
    }

    if(testAltimeterMeasurement()) {
        std::cout << "[PASS] Altimeter Measurement\n";
        ++passed;
    } else {
        std::cout << "[PASS] Altimeter Measurement\n";
    }

    std::cout << "\nTests passed: "
              << passed << "/4\n";

    return passed == 4 ? 0 : 1;
}
