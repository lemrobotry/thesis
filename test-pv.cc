#include "Application.hh"
#include "PV.hh"

APPLICATION

// Reads the PV from the --lop-file parameter.  Reads a single (source, POS,
// alignment) triple from the input and, for each pair of indices into the
// source permutation, prints the features from the PV that fire for that pair.
class testPV : public Permute::Application {
public:
  testPV () :
    Permute::Application ("test-pv") {}

  int main (const std::vector <std::string> & args) {
    Permute::PV pv;

    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    Permute::Permutation source, pos, target;
    std::istream & in = this -> input ();

    Permute::readPermutationWithAlphabet (source, in);
    Permute::readPermutationWithAlphabet (pos, in);
    target = source;
    Permute::readAlignment (target, in);

    for (size_t i = 0; i < source.size () - 1; ++ i) {
      for (size_t j = i + 1; j < source.size (); ++ j) {
	std::cout << "----------------------------------------------------------------------" << std::endl
		  << "- " << i << " " << j <<std::endl
		  << "----------------------------------------------------------------------" << std::endl;
	std::vector <std::string> features;
	pv.features (features, source, pos, i, j);
	for (std::vector <std::string>::const_iterator phi = features.begin (); phi != features.end (); ++ phi) {
	  std::cout << * phi << std::endl;
	}
      }
    }

    return EXIT_SUCCESS;
  }
} app;
