#ifndef ALTIMETER_H
#define ALTIMETER_H

#include <random>

class Altimeter {
public:
    explicit Altimeter(double noiseStdDev = 2.0,
                       unsigned int seed = 42);

    double measure(double trueAltitudeFt);

    void injectBias(double biasFt);
    void clearFault();

    bool hasFault() const;

private:
    std::mt19937 generator;
    std::normal_distribution<double> noise;

    double bias_ft = 0.0;
    bool faultActive = false;
};

#endif
