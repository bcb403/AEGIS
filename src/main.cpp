
#include "Aircraft.h"
#include "Altimeter.h"
#include "FlightComputer.h"

#include <iomanip>
#include <iostream>

int main() {
    Aircraft aircraft;
    Altimeter altimeter;
    FlightComputer flightComputer;

    constexpr int frequency = 50;
    constexpr int duration = 60;
    constexpr double deltaTime = 1.0 / frequency;
    constexpr int totalSamples = frequency * duration;

    std::cout << std::fixed << std::setprecision(2);

    for (int i = 0; i < totalSamples; ++i) {
        if (i > 0) {
            aircraft.update(deltaTime);
        }

        double timestamp = i * deltaTime;

        flightComputer.sample(
            aircraft,
            altimeter,
            timestamp
        );

        if (i % frequency == 0) {
            const auto& sample =
                flightComputer.getSamples().back();

            std::cout
                << "Time: " << timestamp
                << " s | True Alt: "
                << sample.state.altitude_ft
                << " ft | Measured Alt: "
                << sample.measured_altitude_ft
                << " ft\n";
        }
    }

    std::cout
        << "\nFlight simulation complete.\n"
        << "Recorded samples: "
        << flightComputer.getSampleCount()
        << "\n";

    return 0;
}
