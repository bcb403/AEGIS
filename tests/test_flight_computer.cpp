
#include "Aircraft.h"
#include "Altimeter.h"
#include "FlightComputer.h"
#include "FlightDataRecorder.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

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


bool testBiasInjection() {
    Altimeter altimeter(0.0);

    double normal = altimeter.measure(10000.0);

    altimeter.injectBias(350.0);

    double faulty = altimeter.measure(10000.0);

    return normal == 10000.0 &&
           faulty == 10350.0 &&
           altimeter.hasFault();
}


bool testFaultRecovery() {
    Altimeter altimeter(0.0);

    altimeter.injectBias(350.0);
    altimeter.clearFault();

    return !altimeter.hasFault() &&
           altimeter.measure(10000.0) == 10000.0;
}


bool testHistoricalSampleIntegrity() {
    Aircraft aircraft;
    Altimeter altimeter(0.0);
    FlightComputer computer;

    computer.sample(aircraft, altimeter, 0.0);

    altimeter.injectBias(350.0);
    computer.sample(aircraft, altimeter, 1.0);

    const auto& samples = computer.getSamples();

    if (samples.size() != 2) {
        return false;
    }

    bool firstSampleHealthy =
        samples[0].measured_altitude_ft == 10000.0 &&
        !samples[0].altimeter_fault_active;

    bool secondSampleFaulty =
        samples[1].measured_altitude_ft == 10350.0 &&
        samples[1].altimeter_fault_active;

    bool aircraftUnchanged =
        samples[0].state.altitude_ft == 10000.0 &&
        samples[1].state.altitude_ft == 10000.0;

    return firstSampleHealthy &&
           secondSampleFaulty &&
           aircraftUnchanged;
}


bool testFdrRoundTrip() {
    Aircraft aircraft;
    Altimeter altimeter(0.0);
    FlightComputer computer;

    computer.sample(aircraft, altimeter, 0.0);

    altimeter.injectBias(350.0);
    computer.sample(aircraft, altimeter, 0.02);

    const std::string filename = "test_roundtrip.fdr";

    if (!FlightDataRecorder::save(
            filename, computer.getSamples())) {
        return false;
    }

    std::vector<FlightSample> recovered;
    bool loaded =
        FlightDataRecorder::load(filename, recovered);

    std::remove(filename.c_str());

    if (!loaded || recovered.size() != 2) {
        return false;
    }

    return recovered[0].timestamp_sec == 0.0 &&
           recovered[0].state.altitude_ft == 10000.0 &&
           recovered[0].measured_altitude_ft == 10000.0 &&
           !recovered[0].altimeter_fault_active &&
           recovered[1].timestamp_sec == 0.02 &&
           recovered[1].state.altitude_ft == 10000.0 &&
           recovered[1].measured_altitude_ft == 10350.0 &&
           recovered[1].altimeter_fault_active;
}


bool testFdrCorruptionDetection() {
    const std::string filename = "test_corrupt.fdr";

    FlightSample sample{};
    sample.timestamp_sec = 0.0;
    sample.state.altitude_ft = 10000.0;

    if (!FlightDataRecorder::save(filename, {sample})) {
        return false;
    }

    std::fstream file(
        filename,
        std::ios::binary | std::ios::in | std::ios::out
    );

    if (!file) {
        std::remove(filename.c_str());
        return false;
    }

    file.seekp(20);
    char corruptByte = static_cast<char>(0xFF);
    file.write(&corruptByte, 1);
    file.close();

    std::vector<FlightSample> recovered;
    bool loaded =
        FlightDataRecorder::load(filename, recovered);

    std::remove(filename.c_str());

    return !loaded;
}


bool testFdrInvalidFile() {
    const std::string filename = "test_invalid.fdr";

    {
        std::ofstream file(filename, std::ios::binary);
        file << "NOT_A_VALID_AEGIS_RECORDING";
    }

    std::vector<FlightSample> recovered;

    bool loaded =
        FlightDataRecorder::load(filename, recovered);

    std::remove(filename.c_str());

    return !loaded;
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
        std::cout << "[FAIL] Altimeter Measurement\n";
    }

    if(testBiasInjection()) {
        std::cout << "[PASS] Bias Injection\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Bias Injection\n";
    }

    if(testFaultRecovery()) {
        std::cout << "[PASS] Fault Recovery\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Fault Recovery\n";
    }

    if(testHistoricalSampleIntegrity()) {
        std::cout << "[PASS] Historical State Integrity\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Historical State Integrity\n";
    }

    if(testFdrRoundTrip()) {
        std::cout << "[PASS] FDR round-trip\n";
        ++passed;
    } else {
        std::cout << "[FAIL] FDR round-trip\n";
    }

    if(testFdrCorruptionDetection()) {
        std::cout << "[PASS] FDR Corruption Detection\n";
        ++passed;
    } else {
        std::cout << "[FAIL] FDR Corruption Detection\n";
    }

    if(testFdrInvalidFile()) {
        std::cout << "[PASS] FDR Invalid File\n";
        ++passed;
    } else {
        std::cout << "[FAIL] FDR Invalid File\n";
    }

    std::cout << "\nTests passed: "
              << passed << "/10\n";

    return passed == 10 ? 0 : 1;
}
