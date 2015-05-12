#include <cmath>
#include <ext/functional>
#include "Application.hh"
#include "Each.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// A functor for the inner loop of feature counting.
class CountFeatures {
private:
  PV & pv_;
public:
  CountFeatures (PV & pv) :
    pv_ (pv)
  {}
  // Adds one to the count of the given feature if it is in the PV.  Doesn't
  // reintroduce features that have been pruned.
  void operator () (const std::string & phi) {
    PV::iterator it = pv_.find (phi);
    if (it != pv_.end ()) {
      (it -> second) += 1.0;
    }
  }
};

// A functor to compute the smoothed log-odds given preserved and total.
class LogOdds {
private:
  double smooth_;
public:
  LogOdds (double smooth) :
    smooth_ (smooth)
  {}
  double operator () (double preserved, double total) const {
    return std::log (preserved + smooth_)
      - std::log (total - preserved + smooth_);
  }
};  

// Assumes the input LOP parameters have total source sentence feature
// counts, as produced by build-pv.  Copies those total counts into a vector,
// then resets the feature weights to zero.  For each sentence in the input,
// counts the number of times that each feature fires and is preserved in the
// target permutation.  Sets the weight of each feature to its smoothed
// log-odds, defined as log ((preserved + 1/2) / (total - preserved + 1/2)).
// Writes out the resulting PV.
class LogOddsPV : public Application {
private:
  static Core::ParameterBool paramDependency;
  bool DEPENDENCY;
  static Core::ParameterFloat paramSmooth;
  double SMOOTH;
public:
  LogOddsPV () :
    Application ("logodds-pv")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramDependency.printShortHelp (out);
    paramSmooth.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    DEPENDENCY = paramDependency (config);
    SMOOTH = paramSmooth (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }
    CountFeatures count_features (pv);
    LogOdds log_odds (SMOOTH);

    // Copy the current Weights into a vector.
    std::vector <WRef> weights;
    std::for_each (pv.begin (), pv.end (),
		   __gnu_cxx::compose1 (each_fun (& weights,
						  & std::vector <WRef>::push_back),
					std::select2nd <PV::value_type> ()));
    // Copy their values into the totals and reset the Weights to zero.
    std::vector <double> totals (weights.begin (), weights.end ());
    std::fill (weights.begin (), weights.end (), 0.0);

    // Count occurrences in the target permutation.
    Permute::Permutation source, pos, labels, target;
    std::vector <int> parents;
    std::istream & in = this -> input ();
    for (int sentence = 0; sentence < SENTENCES && Permute::readPermutationWithAlphabet (source, in); ++ sentence) {
      Permute::readPermutationWithAlphabet (pos, in);
      if (DEPENDENCY) {
	Permute::readParents (parents, in);
	Permute::readPermutationWithAlphabet (labels, in);
      }
      target = source;
      Permute::readAlignment (target, in);

      for (size_t i = 0; i < target.size () - 1; ++ i) {
	for (size_t j = i + 1; j < target.size (); ++ j) {
	  if (target [i] < target [j]) {
	    std::vector <std::string> features;
	    if (DEPENDENCY) {
	      pv.features (features, source, pos, parents, labels, target [i], target [j]);
	    } else {
	      pv.features (features, source, pos, target [i], target [j]);
	    }
	    std::for_each (features.begin (), features.end (), count_features);
	  }
	}
      }
#ifndef NDEBUG
      std::cerr << "Sentence " << sentence << std::endl;
#endif
    }

    // Compute the logodds:
    std::transform (weights.begin (), weights.end (),
		    totals.begin (),
		    weights.begin (),
		    log_odds);

    // Write out parameters.
    this -> writePV (pv);
    
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool LogOddsPV::paramDependency ("dependency", "use dependency features", false);
Core::ParameterFloat LogOddsPV::paramSmooth ("smooth", "smoothing to add to counts", 0.5);
