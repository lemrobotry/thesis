#include "Application.hh"
#include "PV.hh"

APPLICATION

// Reads a PV, which may consist of feature templates only.  If --generate is
// set, for each sentence in the input, computes the set of features that occur
// at each pair (i,j) of indices, and increments the count of each feature.
// Eliminates any feature from the PV which has a count strictly less than the
// --threshold parameter.  Writes out the resulting PV.
class buildPV : public Permute::Application {
private:
  static Core::ParameterBool paramGenerate;
  bool GENERATE;
  static Core::ParameterFloat paramThreshold;
  double THRESHOLD;
public:
  buildPV ():
    Permute::Application ("build-pv") {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramGenerate.printShortHelp (out);
    paramThreshold.printShortHelp (out);
  }

  virtual void getParameters () {
    Permute::Application::getParameters ();
    GENERATE = paramGenerate (config);
    THRESHOLD = paramThreshold (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    Permute::PV pv;

    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    if (GENERATE) {
      Permute::Permutation source, pos, target, labels;
      std::vector <int> parents;
      std::istream & in = this -> input ();

      // Generate features and count occurrences.
      for (int sentence = 0; sentence < SENTENCES && Permute::readPermutationWithAlphabet (source, in); ++ sentence) {
	Permute::readPermutationWithAlphabet (pos, in);
	if (DEPENDENCY) {
	  Permute::readParents (parents, in);
	  Permute::readPermutationWithAlphabet (labels, in);
	}
	target = source;
	Permute::readAlignment (target, in);

	for (size_t i = 0; i < source.size () - 1; ++ i) {
	  for (size_t j = i + 1; j < source.size (); ++ j) {
	    std::vector <std::string> features;
	    if (DEPENDENCY) {
	      pv.features (features, source, pos, parents, labels, i, j);
	    } else {
	      pv.features (features, source, pos, i, j);
	    }
	    for (std::vector <std::string>::const_iterator phi = features.begin (); phi != features.end (); ++ phi) {
	      pv [* phi] += 1.0;
	    }
	  }
	}
#ifndef NDEBUG
	std::cerr << "Sentence " << sentence << std::endl;
#endif
      }
    }

    std::cout << "Found " << pv.size () << " features" << std::endl;

    // Eliminate features with small counts.
    for (Permute::PV::iterator phi = pv.begin (); phi != pv.end (); ) {
      Permute::PV::iterator current = phi; ++ phi;
      if (double (current -> second) < THRESHOLD) {
  	pv.erase (current);
      }
    }

    std::cout << "Kept " << pv.size () << " features" << std::endl;

    // Write out parameters.
    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool buildPV::paramGenerate ("generate", "generate features from the training data?", true);
Core::ParameterFloat buildPV::paramThreshold ("threshold", "the smallest count to allow", 2.0, 0.0);
