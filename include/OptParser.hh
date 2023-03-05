#ifndef VIVICTPP_OPTPARSER_HH_
#define VIVICTPP_OPTPARSER_HH_

#include "VivictPPConfig.hh"
namespace  vivictpp {

class OptParser {
public:
  bool parseOptions(int argc, char **argv);
  int exitCode;
  VivictPPConfig vivictPPConfig;
};

}  // vivictpp


#endif /* VIVICTPP_OPTPARSER_HH_ */
