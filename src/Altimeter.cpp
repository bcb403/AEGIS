#include "Altimeter.h"

Altimeter::Altimeter(double noiseStdDev,
                     unsigned int seed)
    : generator(seed),
      noise(0.0, noiseStdDev) {
}

double Altimeter::measure(double trueAltitudeFt) {
    return trueAltitudeFt + noise(generator);
}
