
#include "FlightInvestigator.h"
#include "FlightDataRecorder.h"

#include <cmath>
#include <utility>

InspectionResult FlightInvestigator::inspectSurrounding(double timestampSec) const {
    InspectionResult result;

    if (samples.empty() || !std::isfinite(timestampSec)) {
        return result;
    }

    for (const FlightSample& sample : samples) {
        if (sample.timestamp_sec == timestampSec) {
            result.exact = &sample;
            return result;
        } 

        if (sample.timestamp_sec < timestampSec) {
            result.before = &sample;
        }

        if (sample.timestamp_sec > timestampSec && result.after == nullptr) {
            result.after = &sample;
        }
    }

    if (result.exact == nullptr &&
        (result.before == nullptr || result.after == nullptr)) {
            result.outOfRange = true;
        }
    return result;
}

bool FlightInvestigator::loadRecording(
    const std::string& filename
) {
    std::vector<FlightSample> recovered;

    if (!FlightDataRecorder::load(filename, recovered)) {
        return false;
    }

    samples = std::move(recovered);
    return true;
}

const FlightSample* FlightInvestigator::inspect(
    double timestampSec
) const {
    if (samples.empty() || !std::isfinite(timestampSec)) {
        return nullptr;
    }

    const FlightSample* closest = &samples.front();

    double smallestDifference =
        std::abs(closest->timestamp_sec - timestampSec);

    for (const auto& sample : samples) {
        double difference =
            std::abs(sample.timestamp_sec - timestampSec);

        if (difference < smallestDifference) {
            smallestDifference = difference;
            closest = &sample;
        }
    }

    return closest;
}

std::size_t FlightInvestigator::getSampleCount() const {
    return samples.size();
}
