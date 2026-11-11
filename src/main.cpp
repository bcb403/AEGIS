
#include "Aircraft.h"
#include "Altimeter.h"
#include "FlightComputer.h"
#include "FlightDataRecorder.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main() {
    Aircraft aircraft;
    Altimeter altimeter;
    FlightComputer flightComputer;

    constexpr int frequency = 50;
    constexpr int duration = 60;
    constexpr double deltaTime = 1.0 / frequency;
    constexpr int totalSamples = frequency * duration;

    constexpr int faultTimeSec = 38;
    constexpr double faultBiasFt = 350.0;

    std::cout << std::fixed << std::setprecision(2);

    for (int i = 0; i < totalSamples; ++i) {
        if (i > 0) {
            aircraft.update(deltaTime);
        }

        double timestamp = i * deltaTime;

        if (i == faultTimeSec * frequency) {
            altimeter.injectBias(faultBiasFt);

            std::cout
                << "\n[EVENT] ALTIMETER BIAS FAULT INJECTED"
                << " AT T+" << timestamp
                << " s\n\n";
        }

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
                << " ft | Sensor: "
                << (sample.altimeter_fault_active
                    ? "FAULT" : "OK")
                << "\n";
        }
    }

    std::cout
        << "\nFlight simulation complete.\n"
        << "Recorded samples: "
        << flightComputer.getSampleCount()
        << "\n";

    const std::string filename = "flight_001.fdr";

    if (!FlightDataRecorder::save(
            filename,
            flightComputer.getSamples())) {
        std::cerr << "[ERROR] Failed to save flight recording\n";
        return 1;
    }

    std::cout << "[FDR] Flight recording saved: "
              << filename << "\n";

    std::vector<FlightSample> recoveredSamples;

    if (!FlightDataRecorder::load(
            filename,
            recoveredSamples)) {
        std::cerr << "[ERROR] Failed to load flight recording\n";
        return 1;
    }

    std::cout << "[FDR] Recording verified and loaded\n";
    std::cout << "[FDR] Recovered samples: "
              << recoveredSamples.size() << "\n";

    return 0;
}
