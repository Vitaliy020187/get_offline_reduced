#include <fstream>

#include "Settings.h"
#include "Event.h"

class App{
  std::ifstream data_file;
  std::ofstream reduced_file;
  Settings settings;
  Event* event;
  int event_number=1;
public:
  App();
  void openDataFile();
  void openReducedFile();
};