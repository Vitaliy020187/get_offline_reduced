#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>

#include "Event.h"

const int s2us = 1000000;   //seconds to microseconds
const int s2ns = 1000000000;//seconds to nanoseconds
//=====================================================================================================================
Event::Event(Settings* settings){
  this->settings = settings;
  sample_array = new double[settings->act_samples_per_rec*settings->num_channels];
  photon = new Photon(settings);
}
//=====================================================================================================================
inline void Event::readBinaryDataFile(std::ifstream& data_file){
  std::uint32_t time_swapped_bytes;
  std::int16_t  amplitude_swapped_bytes;
  std::string line;

  for (int j=0; j<settings->num_channels; j++){
    data_file.read (reinterpret_cast<char*>(&time_swapped_bytes), sizeof(uint32_t));
    for (int i=0; i<settings->act_samples_per_rec; i++){
      data_file.read (reinterpret_cast<char*>(&amplitude_swapped_bytes), sizeof(int16_t));
      sample_array[i+j*settings->act_samples_per_rec]=__builtin_bswap16 (amplitude_swapped_bytes);
      if (sample_array[i+j*settings->act_samples_per_rec]>INT16_MAX){
        sample_array[i+j*settings->act_samples_per_rec]=sample_array[i+j*settings->act_samples_per_rec]-(UINT16_MAX+1);
      }
    }
  }
  time=__builtin_bswap32 (time_swapped_bytes)*1000;
}
//---------------------------------------------------------------------------------------------------------------------
inline void Event::readAsciiDataFile(std::ifstream& data_file){
  for (int channel=0; channel<settings->num_channels; channel++){
    data_file >> time;
    for (int i=0; i<settings->act_samples_per_rec; i++){
      data_file>>sample_array[i+channel*settings->act_samples_per_rec];
    }
  }
  time=time*1000;
}
//---------------------------------------------------------------------------------------------------------------------
void Event::readDataFile(std::ifstream& data_file){
  if (settings->data_file_name.substr(settings->data_file_name.length()-8, settings->data_file_name.length())==".raw_bin") {readBinaryDataFile(data_file);}
  if (settings->data_file_name.substr(settings->data_file_name.length()-8, settings->data_file_name.length())==".raw_asc") {readAsciiDataFile(data_file);}
}
//=====================================================================================================================
inline void Event::setEventNumber(int event_number){
  this->event_number=event_number;
}
//------------------------------------------
inline void Event::calculateBaseline(int channel){
  baseline[channel]=0;
  for (int i=0; i<int(settings->act_samples_per_rec*settings->pretrigger_fraction*settings->baseline_calc_fraction); i++){
    baseline[channel]+=sample_array[i+settings->act_samples_per_rec*channel];
  }
  baseline[channel]/= int(settings->act_samples_per_rec*settings->pretrigger_fraction*settings->baseline_calc_fraction);
}
//------------------------------------------
inline void Event::calculateBaselineStdDev(int channel){
  baseline_std_dev[channel]=0;
  for (int i=0; i<int(settings->act_samples_per_rec*settings->pretrigger_fraction*settings->baseline_calc_fraction); i++){
    baseline_std_dev[channel]+= pow( baseline[channel]-sample_array[i+settings->act_samples_per_rec*channel], 2 );
  }
  baseline_std_dev[channel]=sqrt(baseline_std_dev[channel]/(int(settings->act_samples_per_rec*settings->pretrigger_fraction*settings->baseline_calc_fraction)-1));
}
//-------------------------------------------
inline void Event::calculateEventIntegral(int channel){//EventIntegral is the same what post_trigger_pulse_area is, to get uniformity with older program
  post_trigger_pulse_area[channel]=0;
  for (int i = int(settings->const_range1/(settings->act_time_per_sample*s2ns)); i<int(settings->const_range2/(settings->act_time_per_sample*s2ns)); i++){
    post_trigger_pulse_area[channel]+=baseline[channel]-sample_array[i+settings->act_samples_per_rec * channel];
  }
  post_trigger_pulse_area[channel] *= settings->act_time_per_sample*s2us;
}
//--------------------------------------------
inline void Event::calculateEventSaturation(int channel){
  saturation[channel]=0;
  for (int i=settings->act_samples_per_rec*channel; i<(channel+1)*settings->act_samples_per_rec; i++){
    if(sample_array[i]<=INT16_MIN || sample_array[i]>=INT16_MAX) saturation[channel]=1;
  }
}
//--------------------------------------------
inline void Event::calculateGreatestAbsValues(int channel){
  t_greatest_abs[channel]=settings->act_samples_per_rec*channel;
  ampl_greatest_abs[channel]=sample_array[settings->act_samples_per_rec*channel];
  for (int i=settings->act_samples_per_rec*channel; i<(channel+1)*settings->act_samples_per_rec; i++){
    if(ampl_greatest_abs[channel]>sample_array[i]) {
      ampl_greatest_abs[channel]=sample_array[i];
      t_greatest_abs[channel]=i-settings->act_samples_per_rec*channel;
    }
  }
  ampl_greatest_abs[channel]=std::abs(ampl_greatest_abs[channel]-baseline[channel]);
}
//--------------------------------------------
inline void Event::calculateT10to90(int channel){
  t10to90[channel]=0;
  for (int i=0; ampl_greatest_abs[channel]*0.1<baseline[channel]-sample_array[t_greatest_abs[channel]-i]; i++ ){
    if(ampl_greatest_abs[channel]*0.9>baseline[channel]-sample_array[t_greatest_abs[channel]-i]){
      t10to90[channel]++;
    }
  }
}
//--------------------------------------------
inline void Event::calculateT90to10(int channel){
  t90to10[channel]=0;
  for (int i=0; ampl_greatest_abs[channel]*0.1<baseline[channel]-sample_array[t_greatest_abs[channel]+i] && t_greatest_abs[channel]+i<(channel+1)*settings->act_samples_per_rec; i++ ){
    if(ampl_greatest_abs[channel]*0.9>baseline[channel]-sample_array[t_greatest_abs[channel]+i]) t90to10[channel]++;
  }
}
//--------------------------------------------
void Event::calculatePhotonsList(int channel){
  for (int sample = settings->act_samples_per_rec * channel + settings->pulse_search_start / (settings->act_time_per_sample * s2ns); sample<(channel+1)*settings->act_samples_per_rec; sample++){
    if (baseline[channel]-baseline_std_dev[channel]*settings->threshold>sample_array[sample]) {
      sample = sample + photon->calculatePhoton(baseline[channel], baseline_std_dev[channel], sample_array, channel, sample);
      photon_list[channel].push_back(*photon);
    }
  }
}
//----------------------------------------------------------------------------
void Event::calculateEvent(int event_number){
  setEventNumber(event_number);
  for (int channel=0; channel<settings->num_channels; channel++){
    calculateBaseline(channel);
    calculateBaselineStdDev(channel);
    calculateEventIntegral(channel);
    calculateEventSaturation(channel);
    calculateGreatestAbsValues(channel);
    calculateT10to90(channel);
    calculateT90to10(channel);
    calculatePhotonsList(channel);
  }
}
//====================================================================================================================================================================
void Event::writeEventToFile(std::ofstream& reduced_file){
  if (baseline_std_dev[0]!=0){//dogleg to fix a bug in enother program, generating raw data files
    reduced_file<<event_number<<"\t"<<time<<"\t";
    for (int channel=0; channel<settings->num_channels; channel++){
      reduced_file<<baseline[channel]<<"\t"<<baseline_std_dev[channel]<<"\t"<<ampl_greatest_abs[channel]<<"\t"<<t_greatest_abs[channel]<<"\t"<<t10to90[channel]<<"\t"<<t90to10[channel]<<"\t"<<post_trigger_pulse_area[channel]<<"\t"<<saturation[channel]<<"\t{"; 
      if (!photon_list[channel].empty()){
        while (photon_list[channel].size()>1){
          photon_list[channel].front().writePhotonToFile(reduced_file);
          reduced_file<<" ";
          photon_list[channel].pop_front();
        }
        photon_list[channel].front().writePhotonToFile(reduced_file);
      }
      if (channel < settings->num_channels-1) reduced_file<<"}\t";
      else reduced_file<<"}"<<std::endl;
      photon_list[channel].clear();
    }
  }
}
