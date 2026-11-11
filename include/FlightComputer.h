
#ifndef FLIGHT_COMPUTER_H
#define FLIGHT_COMPUTER_H

#include "Aircraft.h"
#include "Altimeter.h"

#include <cstddef>
#include <vector>

struct FlightSample {
    double timestamp_sec;
    AircraftState state;
    double measured_altitude_ft;
};

class FlightComputer {
public:
    void sample(const Aircraft& aircraft, 
        Altimeter& altimeter,
         double timestamp_sec);

    const std::vector<FlightSample>& getSamples() const;

    std::size_t getSampleCount() const;

private:
    std::vector<FlightSample> samples;
};

#endif