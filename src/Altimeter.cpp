#include "Altimeter.h"

Altimeter::Altimeter(double noiseStdDev,
                     unsigned int seed)
    : generator(seed),
      noise(0.0, noiseStdDev) {
}

double Altimeter::measure(double trueAltitudeFt) {
    double measuredAltitude = trueAltitudeFt + noise(generator);

    if (faultActive) {
        measuredAltitude += bias_ft;
    }

    return measuredAltitude;
}

void Altimeter::injectBias(double biasFt) {
    bias_ft = biasFt;
    faultActive = true;
}

void Altimeter::clearFault() {
    bias_ft = 0.0;
    faultActive = false;
}

bool Altimeter::hasFault() const {
    return faultActive;
}
