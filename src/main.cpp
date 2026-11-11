#include "Aircraft.h"
#include "FlightComputer.h"

#include <iostream>

int main() {
    Aircraft aircraft;
    FlightComputer flightComputer;

    constexpr int simulationDuration = 60;
    constexpr int frequency = 50;
    constexpr double deltaTime = 1.0 / frequency;

    constexpr int totalSamples = simulationDuration * frequency;

     for (int i = 0; i < totalSamples; ++i) {
        if (i > 0) {
            aircraft.update(deltaTime);
        }

        double timestamp = i * deltaTime;
        flightComputer.sample(aircraft, timestamp);

        if (i % frequency == 0) {
            const auto& state = aircraft.getState();

            std::cout
                << "Time: " << timestamp
                << " s | Altitude: " << state.altitude_ft
                << " ft | Airspeed: " << state.airspeed_kts
                << " kts | Heading: " << state.heading_deg
                << " deg\n";
        }
    }

    std::cout
        << "\nFlight simulation complete.\n"
        << "Recorded samples: "
        << flightComputer.getSampleCount()
        << "\n";

    return 0;
}