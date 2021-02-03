#include <fstream>
#include <list>
#include <cstdint>

#include "Photon.h"

const int s2us = 1000000;

Photon::Photon(Settings* settings){
  this->settings=settings;
}
//=================================================================================
inline void Photon::calculateNumSamples(double baseline, double baseline_std_dev, double* array, int channel, int start_sample){
  num_samples=0;
  while (baseline-baseline_std_dev*settings->threshold>array[start_sample+num_samples]&& start_sample+num_samples<(channel+1)*settings->act_samples_per_rec){
    num_samples++;
  }
}
//-----------------------------
inline void Photon::calculateSaturation(double* array, int start_sample){
  saturation=0;
  for (int j=start_sample; j<start_sample+num_samples; j++){
    if (array[j]<=INT16_MIN || array[j]>=INT16_MAX) saturation=1;
  }  
}
//-----------------------------
inline void Photon::calculateMicros(int channel, int start_sample){
  micros=(start_sample - settings->act_samples_per_rec*channel) * settings->act_time_per_sample * s2us;
}
//-----------------------------
inline void Photon::calculateIntegral(double baseline, double* array, int start_sample){
  integral=0;
  for (int j=start_sample; j<start_sample+num_samples; j++){
    integral+=baseline-array[j];
  }
  integral*=settings->act_time_per_sample*s2us;
}
//-----------------------------
inline void Photon::calculateAmplitudesList(double* array, int start_sample){
  amplitudes_list.clear();
  for (int j=start_sample; j<start_sample+num_samples; j++){
    amplitudes_list.push_back(-array[j]);
  }
}
//=================================================================================
int Photon::calculatePhoton(double baseline, double baseline_std_dev, double* array, int channel, int start_sample){
  
  calculateNumSamples(baseline, baseline_std_dev, array, channel, start_sample);
  calculateSaturation(array, start_sample);
  calculateMicros(channel, start_sample);
  calculateIntegral(baseline, array, start_sample);
  calculateAmplitudesList(array, start_sample);
  
  return num_samples;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Photon::writePhotonToFile(std::ofstream& reduced_file){
  reduced_file<<"("<<micros<<" "<<integral<<" "<<num_samples<<" "<<saturation<<" [";
  for (int i=0; i<num_samples-1; i++) {
    reduced_file<<amplitudes_list.front()<<",";
    amplitudes_list.pop_front(); 
  }
  reduced_file<<amplitudes_list.front()<<"])";
}
