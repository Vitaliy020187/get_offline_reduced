#include <fstream>
#include <list>

#include "Settings.h"
#include "Photon.h"

class Event{
    int event_number, t_greatest_abs[2], saturation[2]; 
    unsigned long int time;
    double baseline[2], baseline_std_dev[2], ampl_greatest_abs[2], t10to90[2], t90to10[2], post_trigger_pulse_area[2];//postTriggerPulseArea is event integral
    double* sample_array;
    Settings* settings;
    Photon* photon;
    std::list <Photon> photon_list[2];

    inline void setEventNumber(int);
    inline void calculateBaseline(int);
    inline void calculateBaselineStdDev(int);
    inline void calculateEventIntegral(int);
    inline void calculateEventSaturation(int);
    inline void calculateGreatestAbsValues(int);
    inline void calculateT10to90(int);
    inline void calculateT90to10(int);
    inline void calculatePhotonsList(int);
    inline void openDataFile(std::ifstream&);
    inline void readBinaryDataFile(std::ifstream&);
    inline void readAsciiDataFile(std::ifstream&);

  public:
    Event(Settings*);
    void readDataFile(std::ifstream&);
    void calculateEvent(int);
    void writeEventToFile(std::ofstream&);
};
