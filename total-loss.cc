#include "Application.hh"
#include "LOPChart.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Computes the total loss of the outputs relative to the targets in the given
// input file under the given model.
class TotalLoss : public Application {
private:
  static Core::ParameterBool paramNormalizeLoss;
  bool NORMALIZE_LOSS;
public:
  TotalLoss () :
    Application ("total-loss")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    NORMALIZE_LOSS = paramNormalizeLoss (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramNormalizeLoss.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    Permutation source, target, pos;
    ParseControllerRef controller = this -> parseController (source);

    double total_loss = 0.0;

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();
    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, in);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, in);
      target = source;
      readAlignment (target, in);

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "total-loss"));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos);
      ScorerRef loss = this -> lossScorer (source, target);
      LOPChart chart (source, WINDOW);

      double best_score = scorer -> score (source);
      do {
	chart.permute (controller, scorer);
	ConstPathRef bestPath = chart.getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
      } while (ITERATE_SEARCH && source.changed ());

      double the_loss = loss -> score (source);
      if (NORMALIZE_LOSS) {
	the_loss /= BeforeScorer::binomial (source.size ());
      }
      out << sentence << " "
	  << source.size () << " "
	  << the_loss << std::endl;
      total_loss += the_loss;
    }
    
    out << "Total " << total_loss << std::endl;

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool TotalLoss::paramNormalizeLoss ("normalize", "normalize the loss function to [0,1]", false);
