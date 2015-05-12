#include <tao.h>
#include "Application.hh"
#include "MIRA.hh"

APPLICATION

// Performs MIRA with a factorization of the LOP matrix.  The simple version,
// when --expanded is false, requires MIRA to make every pair of indices (i, j)
// in the target permutation prefer to be in the target order.  When --expanded
// is true, takes all of the positions between i and j into account as well.
// That is, requires MIRA to make the model prefer (i, ... , j) to (j, ... , i)
// rather than just (i,j) to (j,i).
class FactoredMIRA : public Permute::Application {
private:
  static Core::ParameterBool paramExpanded;
  bool EXPANDED;
public:
  FactoredMIRA () :
    Permute::Application ("factored-mira") {}

  virtual void getParameters () {
    Application::getParameters ();
    EXPANDED = paramExpanded (config);
  }

  int main (const std::vector <std::string> & args) {
    int argc = 0;
    char ** argv = NULL;
    PetscInitialize (& argc, & argv, NULL, "help message");
    TaoInitialize (& argc, & argv, NULL, "help message");

    Permute::ParameterVector pv;
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, pv);

    Permute::Permutation source,
      pos (pv.getPOS ());

    int i = 1;
    do {
      std::cerr << "iteration" << std::endl;
      Permute::set (previous, current);

      std::istream & in = this -> input ();
      for (; Permute::readPermutationWithAlphabet (source, in); ++ i) {
	Permute::readPermutation (pos, in);
	Permute::readAlignment (pos, in);

	std::cerr << source << std::endl;

	// Initialize the vector of Delta(l, r).
	std::vector <SparseParameterVector> delta;
	for (Permutation::const_iterator l = pos.begin (); l != -- pos.end (); ++ l) {
	  for (Permutation::const_iterator r = l + 1; r != pos.end (); ++ r) {
	    delta.push_back (SparseParameterVector ());
	    delta.back ().build (pv, pos.label (* l), pos.label (* r), (* r) - (* l), (* l) < (* r));
	    if (EXPANDED) {
	      for (Permutation::const_iterator m = l + 1; m != r; ++ m) {
		delta.back ().build (pv, pos.label (* l), pos.label (* m), (* m) - (* l), (* l) < (* m));
		delta.back ().build (pv, pos.label (* m), pos.label (* r), (* r) - (* m), (* m) < (* r));
		delta.back ().setMargin (2.0 * (r - l) - 1.0);
	      }
	    }
	    // Remove instances that are already satisfied.
	    // NOTE: This may not behave correctly.
	    if (delta.back ().margin () > 0.0) {
	      delta.pop_back ();
	    }
	  }
	}

	Permute::MIRA (delta);

	update (weightSum, pv);
      }
      Permute::set (current, weightSum, 1.0 / i);
    } while (! this -> converged (previous, current));

    Permute::set (pv, weightSum, 1.0 / i);
    this -> outputParameters (pv);

    TaoFinalize ();
    PetscFinalize ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool FactoredMIRA::paramExpanded ("expanded", "use the expanded MIRA constraints", false); 
