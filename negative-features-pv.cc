#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Prints each feature that has a negative weight.
class NegativeFeatures : public Application {
public:
  NegativeFeatures () :
    Application ("negative-features-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    for (PV::const_iterator it = pv.begin (); it != pv.end (); ++ it) {
      if (double (it -> second) < 0) {
	std::cout << '"' << pv.uncompress (it -> first) << '"'
		  << " " << double (it -> second)
		  << std::endl;
      }
    }

    return EXIT_SUCCESS;
  }
} app;
