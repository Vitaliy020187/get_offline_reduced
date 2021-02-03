#include <fstream>
#include <iostream>

#include "App.h"

App::App(){
  openDataFile();
  openReducedFile();
  event = new Event(&settings);
  while (!data_file.eof()){
    event->readDataFile(data_file);
    event->calculateEvent(event_number);
    event->writeEventToFile(reduced_file);
    event_number++;
  }
  std::cout<<settings.data_file_name.substr(0, settings.data_file_name.length()-8)+"_offline.reduced_photons generated successfully"<<std::endl;
}
//-----------------------------------------------------------------------------------------------------------------
void App::openDataFile(){
	if (settings.data_file_name.substr(settings.data_file_name.size()-8, settings.data_file_name.size())==".raw_bin")
		data_file.open (settings.data_file_name, std::ios::binary);
	if (settings.data_file_name.substr(settings.data_file_name.size()-8, settings.data_file_name.size())==".raw_asc")
		data_file.open (settings.data_file_name);
	if (!data_file.is_open()) {std::cout<<"wrong file name or directory"<<std::endl; exit(EXIT_FAILURE);}
}
//-----------------------------------------------------------------------------------------------------------------
void App::openReducedFile(){
	reduced_file.open (settings.data_file_name.substr(0, settings.data_file_name.length()-8)+"_offline.reduced_photons");
	reduced_file<<"Event number\tTime (micros)\tCh 0 Baseline (V)\tCh 0 Baseline Std Dev (V)\tCh 0 Ampl Greatest Abs (V)\tCh 0 t Greatest Abs (s)\tCh 0 t10-90 (s)\tCh 0 t90-10 (s)\tCh 0 Post-trigger Pulse Area (V*s)\tCh 0 Saturation Test\tCh 0 Photon sample_Array {(micros integral numSamples saturation [sign-flipped amplitudes_sample_array])}";
	if (settings.num_channels>1){
		reduced_file<<"\tCh 1 Baseline (V)\tCh 1 Baseline Std Dev (V)\tCh 1 Ampl Greatest Abs (V)\tCh 1 t Greatest Abs (s)\tCh 1 t10-90 (s)\tCh 1 t90-10 (s)\tCh 1 Post-trigger Pulse Area (V*s)\tCh 1 Saturation Test\tCh 1 Photon sample_Array {(micros integral numSamples saturation [sign-flipped amplitudes_sample_array])}";
	}
	reduced_file<<"\tthreshold_in_baseline_std_dev_num="<<settings.threshold<<"\tpulse_search_start(ns)="<<settings.pulse_search_start<<"\tconst_integration_range(ns)"<<settings.const_range1<<'\t'<<settings.const_range2<<std::endl;
}
