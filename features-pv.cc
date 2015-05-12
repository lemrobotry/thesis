#include "Application.hh"
#include "Each.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Reads a PV, which should consist of feature templates only.  Generates a list
// of features that occur in the input.
class FeaturesPV : public Application {
public:
  FeaturesPV () :
    Application ("features-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;

    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    Permutation source, pos, target;
    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, in);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, in);
      target = source;
      readAlignment (target, in);

      for (size_t i = 0; i < source.size () - 1; ++ i) {
	for (size_t j = i + 1; j < source.size (); ++ j) {
	  std::vector <std::string> features;
	  pv.features (features, source, pos, i, j);
	  std::transform (features.begin (), features.end (),
			  std::ostream_iterator <std::string> (out, "\n"),
			  each_fun (& pv, & PV::uncompress));
	}
      }
#ifndef NDEBUG
      std::cerr << "Sentence " << sentence << std::endl;
#endif
    }

    return EXIT_SUCCESS;
  }
} app;
