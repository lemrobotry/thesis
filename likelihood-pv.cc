#include "Application.hh"
#include "GradientChart.hh"
#include "ParseController.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Computes the likelihood of the given input file under the given model.
class LikelihoodPV : public Application {
public:
  LikelihoodPV () :
    Application ("likelihood-pv")
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

    Permutation source, target, pos;

    ParseControllerRef controller (CubicParseController::create ());

    std::istream & input = this -> input ();

    double likelihood = 0.0;

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      target = source;
      readAlignment (target, input);

      // GradientChart overwrites the weights with their gradients every
      // iteration, so this resets them from a copy of their values.
      std::copy (values.begin (), values.end (),
		 weights.begin ());

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "LikelihoodPV"));
      this -> sumBeforeCost (bc, pv, source, pos);
      GradientScorer scorer (bc, target);
      double numerator = scorer.score (target);

      GradientChart chart (target);
      chart.parse (controller, scorer, pv);
      double denominator = chart.Z ();

      std::cerr << sentence << " "
		<< source.size () << " "
		<< numerator << " "
		<< denominator << std::endl;
      likelihood += numerator - denominator;
    }

    std::cout << "Log likelihood: " << likelihood << std::endl;

    return EXIT_SUCCESS;
  }
} app;
