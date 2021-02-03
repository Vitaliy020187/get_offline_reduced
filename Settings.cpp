#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include "Settings.h"

const int s2ns = 1000000000;//seconds to nanoseconds
//===================================================================================================================
Settings::Settings(){
  readInitFile();
  readSetupFile();
  checkInputValues();
}
//---------------------------------------------------------------------------------------------------------------------------
inline void Settings::readInitFile(){
  std::string buf;
  std::stringstream buf1;
  try{
    std::ifstream init_file("init.txt");
    if (!init_file.is_open()) {throw "file init.txt is not found";}
    getline( init_file, buf );
    data_file_name = buf.substr(buf.find('\t')+1, buf.length());
    init_file>>buf>>buf;
    baseline_calc_fraction=stof(buf);
    init_file>>buf>>buf;
    threshold=stof(buf);
    init_file>>buf>>buf;
    pulse_search_start=stof(buf);
    init_file>>buf>>buf;
    const_range1=stof(buf);
    init_file>>buf;
    const_range2=stof(buf);
    init_file>>buf;
    getline( init_file, buf);
    buf1<<buf;

    while (!buf1.eof()){
      buf1>>buf;
      if (stoi(buf)>0){
        single_trace_number_list.push_back(stoi(buf));
      }
    }
    sort(single_trace_number_list.begin(), single_trace_number_list.end());
    if (single_trace_number_list.empty()) single_trace_number_list.push_back(0);
    single_trace_number_list.erase( unique( single_trace_number_list.begin(), single_trace_number_list.end() ), single_trace_number_list.end() );
    init_file.close();
  }
  catch (std::invalid_argument& e){
    std::cerr<<"invalid argument input in init.txt"<<std::endl;
    exit(EXIT_FAILURE);
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
  }
}
//------------------------------------------------------------------------------------------------------------------------
inline void Settings::readSetupFile(){
  std::string buf;
  try{
    std::ifstream setup_file (data_file_name.substr(0, data_file_name.find('.')+1)+"setup");
    if (!setup_file.is_open()) {throw "file *.setup is not found";}

    std::string find_in_buf="Actual samples per record :\t";
    while (buf.find(find_in_buf)==std::string::npos && !setup_file.eof()){
      std::getline(setup_file, buf);
    }
    act_samples_per_rec=stoi(buf.substr(find_in_buf.size(), buf.size()));

    find_in_buf="Actual time per sample (s) :\t";
    while (buf.find(find_in_buf)==std::string::npos && !setup_file.eof()){
      std::getline(setup_file, buf);
    }
    act_time_per_sample=stod(buf.substr(find_in_buf.size(), buf.size()));

    find_in_buf="Pretigger fraction :\t";
    while (buf.find(find_in_buf)==std::string::npos && !setup_file.eof()){
      std::getline(setup_file, buf);
    }
    pretrigger_fraction=stod(buf.substr(find_in_buf.size(), buf.size()));

    find_in_buf="C0 Volts per ADU (V) :\t";
    while (buf.find(find_in_buf)==std::string::npos && !setup_file.eof()){
      std::getline(setup_file, buf);
    }
    Volts_per_ADU[0]=stod(buf.substr(find_in_buf.size(), buf.size()));


    while (!setup_file.eof()){
      std::getline(setup_file, buf);
      if (buf.find("C1 ON")!=std::string::npos) {
        num_channels = 2;
        find_in_buf="C1 Volts per ADU (V) :\t";
        while (buf.find(find_in_buf)==std::string::npos && !setup_file.eof()){
          std::getline(setup_file, buf);
        }
        Volts_per_ADU[1]=stod(buf.substr(find_in_buf.size(), buf.size()));
      }
    }
    setup_file.close();
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
  }
}
//-----------------------------------------------------------------------------------------------------------------------
inline void Settings::checkInputValues(){
  try{
    if (baseline_calc_fraction<=0 || baseline_calc_fraction>1) throw "ivalid baseline_calc_fraction value, must be >0 and <=1";
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  try{
    if (threshold<=0) throw "ivalid threshold value, must be >0";
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  try{
    if (const_range1<0) throw "ivalid const_range1 value, will be set to 0";
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    const_range1=0;
  }
  try{
    if (const_range2>act_samples_per_rec*act_time_per_sample*s2ns)
      throw "ivalid const_range2 value, will be set to act_samples_per_rec*act_time_per_sample (ns)";
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    const_range2=act_samples_per_rec*act_time_per_sample*s2ns;
  }
  try{
    if (const_range1>=const_range2)
      throw "ivalid const_range1 const_range2 values, must be const_range1<const_range2, will be set const_range1 = act_time_per_rec*pretrigger_fraction, const_range2 =  act_time_per_rec (ns)";
  }
  catch (const char* msg){
    std::cerr << msg << std::endl;
    const_range1=act_samples_per_rec*act_time_per_sample*s2ns*pretrigger_fraction;
    const_range2=act_samples_per_rec*act_time_per_sample*s2ns;
  }
}