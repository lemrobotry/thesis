#include <Core/Utility.hh>
#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Defines a strict weak ordering on FeatureTemplate by the number of active
// primitive features.
struct FeatureTemplateActive {
  bool operator () (const FeatureTemplate & t1, const FeatureTemplate & t2) const {
    return t1.active () < t2.active ();
  }
};

// Reads an empty PV (feature templates, but no features).  Creates a new PV and
// adds the templates from the empty PV one at a time, collecting features from
// the input data, counting occurrences of each instance of the template, and
// copying those features that pass the threshold back to the original PV.
class IncrementalPV : public Application {
private:
  static Core::ParameterFloat paramThreshold;
  double THRESHOLD;
public:
  IncrementalPV () :
    Application ("incremental-pv")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramThreshold.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    THRESHOLD = paramThreshold (config);
  }

  bool generalizes (const PV & pv,
		    const std::vector <FeatureTemplate> & tv,
		    const std::string & feature) const;

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;

    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::cerr << "THRESHOLD: " << THRESHOLD << std::endl
	      << "DEPENDENCY: " << DEPENDENCY << std::endl;

    // Extracts the feature templates from pv and sorts them by the number of
    // primitive features.
    const TemplateList & tl (pv.templates ());
    std::vector <FeatureTemplate> tv;
    std::transform (tl.begin (), tl.end (),
		    std::back_inserter (tv),
		    std::select1st <std::pair <FeatureTemplate, std::string> > ());
    std::sort (tv.begin (), tv.end (), FeatureTemplateActive ());

    // Iterates over feature templates.  Puts each into a new PV one at a time
    // and collects feature instances from the input.
    for (std::vector <FeatureTemplate>::const_iterator it = tv.begin ();
	 it != tv.end (); ++ it) {
      PV sub_pv (pv);
      sub_pv.addTemplate (* it);
      std::vector <FeatureTemplate> gen (tl.generalize (* it));
      // For debugging purposes:
      std::cerr << "Template:" << std::endl << * it << std::endl;
      std::cerr << "Generalizations:" << std::endl;
      for (std::vector <FeatureTemplate>::const_iterator gen_it = gen.begin ();
	   gen_it != gen.end (); ++ gen_it) {
	std::cerr << * gen_it << std::endl;
      }
      // End debugging.

      Permutation source, pos, labels, target;
      std::vector <int> parents;
      std::istream & input = this -> input ();
      for (int sentence = 0;
	   sentence < SENTENCES && readPermutationWithAlphabet (source, input);
	   ++ sentence) {
	readPermutationWithAlphabet (pos, input);
	if (DEPENDENCY) {
	  readParents (parents, input);
	  readPermutationWithAlphabet (labels, input);
	}
	target = source;
	readAlignment (target, input);

	for (size_t i = 0; i < source.size () - 1; ++ i) {
	  for (size_t j = i + 1; j < source.size (); ++ j) {
	    std::vector <std::string> features;
	    if (DEPENDENCY) {
	      sub_pv.features (features, source, pos, parents, labels, i, j);
	    } else {
	      sub_pv.features (features, source, pos, i, j);
	    }
	    for (std::vector <std::string>::const_iterator phi = features.begin ();
		 phi != features.end (); ++ phi) {
	      if (this -> generalizes (pv, gen, sub_pv.uncompress (* phi))) {
		sub_pv [* phi] += 1.0;
	      }
	    }
	  }
	}
      }
      // Copies features back to the original pv, but eliminates those with
      // counts below the threshold.
      for (PV::const_iterator phi = sub_pv.begin (); phi != sub_pv.end (); ++ phi) {
	if (double (phi -> second) >= THRESHOLD) {
	  pv.getParameter (sub_pv.uncompress (phi -> first)) = double (phi -> second);
	}
      }
    }

    std::cout << "Kept " << pv.size () << " features" << std::endl;

    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;
    
Core::ParameterFloat IncrementalPV::paramThreshold ("threshold", "the smallest count to allow", 2.0, 0.0);

bool IncrementalPV::generalizes (const PV & pv,
				 const std::vector <FeatureTemplate> & tv,
				 const std::string & feature) const {
  FeatureMap fm;
  fm.setAll (feature);
  pv.templates ().intern (fm);
  for (std::vector <FeatureTemplate>::const_iterator it = tv.begin ();
       it != tv.end (); ++ it) {
    if (pv.find (pv.templates ().featureFromTemplate (fm, * it)) == pv.end ()) {
      return false;
    }
  }
  return true;
}
