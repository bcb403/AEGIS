#include "FlightInvestigator.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

void printSample(const FlightSample* sample) {
    if (sample == nullptr) {
        return;
    }

    double error =
        sample->measured_altitude_ft -
        sample->state.altitude_ft;

    std::cout
        << "\n--- FLIGHT SAMPLE ---\n"
        << "Timestamp:         "
        << sample->timestamp_sec << " s\n"
        << "True altitude:     "
        << sample->state.altitude_ft << " ft\n"
        << "Measured altitude: "
        << sample->measured_altitude_ft << " ft\n"
        << "Altitude error:    "
        << std::showpos << error
        << std::noshowpos << " ft\n"
        << "Sensor fault:      "
        << (sample->altimeter_fault_active
            ? "ACTIVE" : "NONE")
        << "\n";
}

int main(int argc, char* argv[]) {
    const std::string filename =
        argc > 1 ? argv[1] : "flight_001.fdr";

    FlightInvestigator investigator;

    if (!investigator.loadRecording(filename)) {
        std::cerr
            << "[ERROR] Could not load recording: "
            << filename << "\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n=== AEGIS INVESTIGATOR ===\n";
    std::cout << "Recording: " << filename << "\n";
    std::cout << "Samples: "
              << investigator.getSampleCount() << "\n";
    std::cout << "Enter a timestamp in seconds.\n";
    std::cout << "Type 'exit' to quit.\n\n";

    std::string input;

    while (true) {
        std::cout << "AEGIS> ";

        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "exit") {
            break;
        }

        std::istringstream parser(input);
        double timestamp;
        std::string extra;

        if (!(parser >> timestamp) || (parser >> extra)) {
            std::cout
                << "Invalid timestamp. Try 38.00\n";
            continue;
        }

        InspectionResult result =
            investigator.inspectSurrounding(timestamp);


        const FlightSample* sample = nullptr;

        if (result.exact != nullptr) {
            std::cout << "Exact timestamp found!\n";
            sample = result.exact;
        } else if (result.before != nullptr) {
            sample = result.before;
        } else if (result.after != nullptr) {
            sample = result.after;
        }

        if (sample == nullptr) {
            std::cout << "No sample available.\n";
            continue;
        }

        if (result.outOfRange) {
            std::cout
                << "Requested timestamp is outside "
                << "the recording range.\n";
            std::cout << "Displaying nearest available boundary sample:\n";
            if (result.before != nullptr) {
                printSample(result.before);
            } else if (result.after != nullptr) {
                printSample(result.after);
            }
        } else if (result.exact != nullptr) {
            std::cout << "\nExact match:\n";
            printSample(result.exact);
        } else {
            std::cout << "\nNo exact match. Displaying surrounding samples:\n";
            if (result.before != nullptr) {
                printSample(result.before);
            }

            if(result.after != nullptr) {
                printSample(result.after);
            }
        }
    }

    std::cout << "Investigation complete.\n";
    return 0;
}
