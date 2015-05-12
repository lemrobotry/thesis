#include "Application.hh"
#include "PV.hh"

APPLICATION

// Reads each of the PV files indicated on the command line into a single PV and
// writes the result to a new PV file.
class mergePV : public Permute::Application {
public:
  mergePV () :
    Permute::Application ("merge-pv") {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    Permute::PV pv;

    for (std::vector <std::string>::const_iterator file = args.begin (); file != args.end (); ++ file) {
      Permute::readFile (pv, * file);
    }

    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;
