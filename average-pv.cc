#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Accepts many model files on the command line and outputs the model with the
// average feature weight.
class AveragePV : public Application {
public:
  AveragePV () :
    Application ("average-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    int count = 0;

    std::vector <std::string>::const_iterator it = args.begin ();
    if (it == args.end ()) {
      return EXIT_SUCCESS;
    }
    if (! readFile (pv, * it)) {
      std::cerr << "Could not read LOP parameter file: " << * it << std::endl;
      return EXIT_FAILURE;
    } else {
      count = 1;
    }

    // Successfully read the first parameter file.  Now reads additional
    // parameter files, aggregating their parameters into the existing PV.
    for (++ it; it != args.end (); ++ it) {
      if (! aggregateFile (pv, * it)) {
	std::cerr << "Could not read LOP parameter file: " << * it << std::endl;
	return EXIT_FAILURE;
      } else {
	++ count;
      }
    }

    // Normalizes the parameter totals by count.
    for (PV::iterator pv_it = pv.begin (); pv_it != pv.end (); ++ pv_it) {
      pv_it -> second /= count;
    }

    // Writes out the average PV.
    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;
