
#ifndef FLIGHT_COMPUTER_H
#define FLIGHT_COMPUTER_H

#include "Aircraft.h"

#include <cstddef>
#include <vector>

struct FlightSample {
    double timestamp_sec;
    AircraftState state;
};

class FlightComputer {
public:
    void sample(const Aircraft& aircraft, double timestamp_sec);

    const std::vector<FlightSample>& getSamples() const;

    std::size_t getSampleCount() const;

private:
    std::vector<FlightSample> samples;
};

#endif