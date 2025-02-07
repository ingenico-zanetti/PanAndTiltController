#include "GlobalConfiguration.hpp"

static const char *const FW_VERSION = "Pan & Tilt Controller v1.0.1, built on " __DATE__ " at " __TIME__;

const char *getFWVersion(void){
  return(FW_VERSION);
}
