#pragma once

#include <list>

#include "Settings.h"

class Photon{//contains data of single pulse representing single photon
    float micros, integral; // photon arrival time in microsec, photon integral
    int num_samples, saturation; //pulse width in samples, is digitizer saturated, max inverted amplitude in samples of photon, sample where is max
    std::list <int> amplitudes_list;
    Settings* settings;
    inline void calculateNumSamples(double, double, double*, int, int);
    inline void calculateSaturation(double*, int);
    inline void calculateMicros(int, int);
    inline void calculateIntegral(double, double*, int);
    inline void calculateAmplitudesList(double*, int);
    
  public:
    Photon(Settings*);
    int calculatePhoton(double, double, double*, int, int);
    void writePhotonToFile(std::ofstream&);// write to file *offline.redused_integral
};
