#pragma once

#include <string>
#include <list>
#include <vector>
class Settings{
	inline void readInitFile();
	inline void readSetupFile();
	inline void checkInputValues();
  public:
	int num_channels=1, act_samples_per_rec;
	std::vector <int> single_trace_number_list;
	float threshold, baseline_calc_fraction, pulse_search_start=0, const_range1, const_range2;
	double act_time_per_sample, pretrigger_fraction, Volts_per_ADU[2];
	std::string data_file_name;
    Settings();
    
};