#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Reads each PV listed in the command-line arguments.  For each feature that
// occurs in the first PV, prints its weight from each of the PVs in a format
// that can be understood by R's read.table function.
class RPVs : public Application {
public:
  RPVs () :
    Application ("r-pvs")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::vector <PV> pvs;
    for (std::vector <std::string>::const_iterator a = args.begin (); a != args.end (); ++ a) {
      std::cout << (* a ++) << '\t';
      pvs.push_back (PV ());
      this -> LOP_FILE = * a;
      this -> readPV (pvs.back ());
    }
    std::cout << std::endl;

    int i = 0;
    PV & pv = pvs.front ();
    for (PV::const_iterator phi = pv.begin (); phi != pv.end (); ++ phi) {
      std::cout << (++ i) << '\t' << double (phi -> second);
      for (std::vector <PV>::iterator v = pvs.begin () + 1; v != pvs.end (); ++ v) {
	std::cout << '\t' << double ((* v) [phi -> first]);
      }
      std::cout << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
