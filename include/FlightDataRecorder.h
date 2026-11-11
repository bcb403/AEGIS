#ifndef FLIGHT_DATA_RECORDER_H
#define FLIGHT_DATA_RECORDER_H

#include "FlightComputer.h"

#include <string>
#include <vector>

class FlightDataRecorder {
public:
    static bool save(
        const std::string& filename,
        const std::vector<FlightSample>& samples
    );

    static bool load(
        const std::string& filename,
        std::vector<FlightSample>& samples
    );
};

#endif
