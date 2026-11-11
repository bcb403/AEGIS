#include "FlightComputer.h"

void FlightComputer::sample(
    const Aircraft& aircraft,
    double timestamp_sec
) {
    FlightSample newSample{
        timestamp_sec,
        aircraft.getState()
    };

    samples.push_back(newSample);
}

const std::vector<FlightSample>&
FlightComputer::getSamples() const {
    return samples;
}

std::size_t FlightComputer::getSampleCount() const {
    return samples.size();
}