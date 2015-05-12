#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Prints each (feature, weight) pair from a PV in a format that can be read by
// R's read.table function.
class RPV : public Application {
public:
  RPV () :
    Application ("r-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    int i = 0;
    std::cout << "feature" << '\t' << "weight" << std::endl;
    for (PV::const_iterator phi = pv.begin (); phi != pv.end (); ++ phi) {
      std::cout << ++ i
		<< '\t'
		<< '"' << phi -> first << '"'
		<< '\t'
		<< double (phi -> second)
		<< std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
