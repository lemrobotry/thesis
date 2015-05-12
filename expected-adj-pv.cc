#include "Application.hh"
#include "AdjacentLoss.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Computes the expected number of preserved adjacencies of the given input file
// under the given model.
class ExpectedAdjacency : public Application {
public:
  ExpectedAdjacency () :
    Application ("expected-adj-pv")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <WRef> weights (pv.size ());
    std::transform (pv.begin (), pv.end (),
		    weights.begin (),
		    std::select2nd <PV::value_type> ());
    std::vector <double> values (weights.begin (), weights.end ());

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    ParseControllerRef controller = this -> parseController (target);

    std::istream & input = this -> input ();

    double expected_adj = 0.0;

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

      // AdjacentGradientChart overwrites the weights with their gradients every
      // iterations, so this resets them from a copy of their values.
      std::copy (values.begin (), values.end (),
		 weights.begin ());

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "ExpectedAdjacency"));
      this -> sumBeforeCost (bc, pv, source, pos, parents, labels);
      ExpectationGradientScorer scorer (bc, target);

      AdjacentGradientChart chart (target);
      chart.parse (controller, scorer, pv);

      std::cerr << sentence << " "
		<< source.size () << " "
		<< chart.Z () << std::endl;
      expected_adj += chart.Z ().expectation ();
    }

    std::cout << "Total expected adjacencies: " << expected_adj << std::endl;

    return EXIT_SUCCESS;
  }
} app;
