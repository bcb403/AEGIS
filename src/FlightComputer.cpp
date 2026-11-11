#include "FlightComputer.h"

void FlightComputer::sample(
    const Aircraft& aircraft,
    Altimeter& altimeter,
    double timestamp_sec

) {
    const AircraftState& state = aircraft.getState();

    double measuredAltitude =
        altimeter.measure(state.altitude_ft);

    FlightSample newSample{
        timestamp_sec,
        aircraft.getState(),
        measuredAltitude
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