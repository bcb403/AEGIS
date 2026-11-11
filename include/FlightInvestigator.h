#ifndef FLIGHT_INVESTIGATOR_H
#define FLIGHT_INVESTIGATOR_H

#include "FlightComputer.h"

#include <cstddef>
#include <string>
#include <vector>

struct InspectionResult {
    const FlightSample* exact = nullptr;
    const FlightSample* before = nullptr;
    const FlightSample* after = nullptr;
    bool outOfRange = false;

};

class FlightInvestigator {
public:
    InspectionResult inspectSurrounding(double timestampSec) const;

    bool loadRecording(const std::string& filename);

    const FlightSample* inspect(double timestampSec) const;

    std::size_t getSampleCount() const;

private:
    std::vector<FlightSample> samples;
};

#endif
