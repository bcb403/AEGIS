#ifndef AIRCRAFT_H
#define AIRCRAFT_H

struct AircraftState {
    double altitude_ft = 10000.0;
    double airspeed_kts = 180.0;
    double heading_deg = 90.0;
    double pitch_deg = 0.0;
    double roll_deg = 0.0;
    double vertical_speed_fpm = 0.0;
};

class Aircraft {
    public:
        Aircraft();
        
        const AircraftState& getState() const;

        void update(double deltaTime);

    private:
        AircraftState state;
};

#endif