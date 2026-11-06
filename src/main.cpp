#include "Aircraft.h"

#include <iostream>

int main() {
    Aircraft aircraft;

    constexpr double simulationDuration = 60.0;
    constexpr double frequency = 50.0;
    constexpr double deltaTime = 1.0 / frequency;

    constexpr int totalSamples =
        static_cast<int>(simulationDuration * frequency);

    for (int sample = 0; sample < totalSamples; ++sample) {
        aircraft.update(deltaTime);

        if (sample % 50 == 0) {
            const AircraftState& state = aircraft.getState();

            std::cout
                << "Time: " << sample * deltaTime
                << " s | Altitude: " << state.altitude_ft
                << " ft | Airspeed: " << state.airspeed_kts
                << " kts | Heading: " << state.heading_deg
                << " deg\n";
        }
    }

    std::cout << "Flight simulation complete. "
              << totalSamples << "samples processed.\n";

    return 0;
}