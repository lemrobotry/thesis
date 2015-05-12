#include "Application.hh"
#include "Iterator.hh"
#include "LOPChart.hh"
#include "Loss.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Decodes the input using a PV.
class DecodePV : public Application {
public:
  DecodePV () :
    Application ("decode-pv") {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::cerr << "ITERATE_SEARCH: " << Boolean (ITERATE_SEARCH) << std::endl;

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

//     ChartFactoryRef factory = ChartFactory::create ();
    Permutation source, target, pos, labels;
    std::vector <int> parents;
    ParseControllerRef controller = this -> parseController (source);

    Loss loss;

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();
    while (readPermutationWithAlphabet (source, in)) {
      readPermutationWithAlphabet (pos, in);
      if (DEPENDENCY) {
	readParents (parents, in);
	readPermutationWithAlphabet (labels, in);
      }
      target = source;
      readAlignment (target, in);

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "decode-pv"));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos, parents, labels);
//       ChartRef chart = factory -> chart (source, WINDOW);
      LOPChart chart (source, WINDOW);

      double best_score = scorer -> score (source);
      do {
// 	Chart::permute (chart, controller, scorer);
// 	ConstPathRef bestPath = chart -> getBestPath ();
	chart.permute (controller, scorer);
	ConstPathRef bestPath = chart.getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
      } while (ITERATE_SEARCH && source.changed ());

      out << source << std::endl;
      loss.add (target, source);
    }

    std::cerr << loss << std::endl;

    return EXIT_SUCCESS;
  }
} app;
