
#include "Aircraft.h"
#include "Altimeter.h"
#include "FlightComputer.h"
#include "FlightDataRecorder.h"
#include "FlightInvestigator.h"

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


bool testInvestigatorLoadsRecording() {
    const std::string filename = "test_investigator.fdr";

    FlightSample sample{};
    sample.timestamp_sec = 0.0;
    sample.state.altitude_ft = 10000.0;

    if (!FlightDataRecorder::save(filename, {sample})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    return loaded && investigator.getSampleCount() == 1;
}


bool testInvestigatorTimestamp() {
    const std::string filename = "test_timestamp.fdr";

    FlightSample first{};
    first.timestamp_sec = 10.0;
    first.state.altitude_ft = 10000.0;

    FlightSample second{};
    second.timestamp_sec = 20.0;
    second.state.altitude_ft = 9500.0;

    if (!FlightDataRecorder::save(
            filename, {first, second})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    const FlightSample* sample =
        investigator.inspect(19.8);

    return sample != nullptr &&
           sample->timestamp_sec == 20.0 &&
           sample->state.altitude_ft == 9500.0;
}


bool testInvestigatorFault() {
    const std::string filename = "test_fault.fdr";

    FlightSample healthy{};
    healthy.timestamp_sec = 37.0;
    healthy.state.altitude_ft = 10000.0;
    healthy.measured_altitude_ft = 10000.0;
    healthy.altimeter_fault_active = false;

    FlightSample faulty{};
    faulty.timestamp_sec = 38.0;
    faulty.state.altitude_ft = 10000.0;
    faulty.measured_altitude_ft = 10350.0;
    faulty.altimeter_fault_active = true;

    if (!FlightDataRecorder::save(
            filename, {healthy, faulty})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    const FlightSample* sample =
        investigator.inspect(38.0);

    return sample != nullptr &&
           sample->altimeter_fault_active &&
           sample->measured_altitude_ft -
               sample->state.altitude_ft == 350.0;
}

bool testInvestigatorSurrounding() {
    const std::string filename = "test_surrounding.fdr";

    FlightSample first{};
    first.timestamp_sec = 1.00;

    FlightSample second{};
    second.timestamp_sec = 1.02;

    FlightSample third{};
    third.timestamp_sec = 1.04;

    if (!FlightDataRecorder::save(
            filename, {first, second, third})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    InspectionResult result =
        investigator.inspectSurrounding(1.03);

    return result.exact == nullptr &&
            result.before != nullptr &&
            result.before->timestamp_sec == 1.02 &&
            result.after != nullptr &&
            result.after->timestamp_sec == 1.04 &&
            result.outOfRange == false;
}

bool testInvestigatorExactMatch() {
    const std::string filename = "test_exact_match.fdr";

    FlightSample first{};
    first.timestamp_sec = 1.00;

    FlightSample second{};
    second.timestamp_sec = 1.02;

    FlightSample third{};
    third.timestamp_sec = 1.04;

    if (!FlightDataRecorder::save(
            filename, {first, second, third})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    InspectionResult result =
        investigator.inspectSurrounding(1.02);

    return result.exact != nullptr &&
            result.exact->timestamp_sec == 1.02 &&
            result.before != nullptr &&
            result.before->timestamp_sec == 1.00 &&
            result.after == nullptr &&
            result.outOfRange == false;
}

bool testInvestigatorLowerBoundary() {
    const std::string filename = "test_lower_boundary.fdr";

    FlightSample first{};
    first.timestamp_sec = 1.00;

    FlightSample second{};
    second.timestamp_sec = 1.02;

    FlightSample third{};
    third.timestamp_sec = 1.04;

    if (!FlightDataRecorder::save(
            filename, {first, second, third})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    InspectionResult result =
        investigator.inspectSurrounding(0.98);

    return result.exact == nullptr &&
            result.before == nullptr &&
            result.after != nullptr &&
            result.after->timestamp_sec == 1.00 &&
            result.outOfRange == true;
}

bool testInvestigatorUpperBoundary() {
    const std::string filename = "test_upper_boundary.fdr";

    FlightSample first{};
    first.timestamp_sec = 1.00;

    FlightSample second{};
    second.timestamp_sec = 1.02;

    FlightSample third{};
    third.timestamp_sec = 1.04;

    if (!FlightDataRecorder::save(
            filename, {first, second, third})) {
        return false;
    }

    FlightInvestigator investigator;
    bool loaded = investigator.loadRecording(filename);

    std::remove(filename.c_str());

    if (!loaded) {
        return false;
    }

    InspectionResult result =
        investigator.inspectSurrounding(1.06);

    return result.exact == nullptr &&
            result.before != nullptr &&
            result.before->timestamp_sec == 1.04 &&
            result.after == nullptr &&
            result.outOfRange == true;
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


    if (testInvestigatorLoadsRecording()) {
        std::cout << "[PASS] Investigator loads recording\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator loads recording\n";
    }

    if (testInvestigatorTimestamp()) {
        std::cout << "[PASS] Investigator timestamp inspection\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator timestamp inspection\n";
    }

    if (testInvestigatorFault()) {
        std::cout << "[PASS] Investigator fault inspection\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator fault inspection\n";
    }

    if (testInvestigatorSurrounding()) {
        std::cout << "[PASS] Investigator surrounding samples\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator surrounding samples\n";
    }

    if (testInvestigatorExactMatch()) {
        std::cout << "[PASS] Investigator Exact Match\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator Exact Match\n";
    }

    if (testInvestigatorLowerBoundary()) {
        std::cout << "[PASS] Investigator Lower Boundary\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator Lower Boundary\n";
    }

    if (testInvestigatorUpperBoundary()) {
        std::cout << "[PASS] Investigator Upper Boundary\n";
        ++passed;
    } else {
        std::cout << "[FAIL] Investigator Upper Boundary\n";
    }


    std::cout << "\nTests passed: "
              << passed << "/17\n";

    return passed == 17 ? 0 : 1;
}
