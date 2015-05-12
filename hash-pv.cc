#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Hash each feature in a PV and print it.
class HashPV : public Application {
public:
  HashPV () :
    Application ("hash-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    StringHash hasher;

    for (PV::const_iterator it = pv.begin (); it != pv.end (); ++ it) {
      const std::string & feature = it -> first;
      std::cout << hasher (feature) << " [";
      std::copy (feature.begin (), feature.end (),
		 std::ostream_iterator <int> (std::cout, " "));
      std::cout << "] " << pv.uncompress (feature) << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
