#include <Core/CompressedStream.hh>

#include <Fsa/AlphabetXml.hh>
#include <Fsa/Best.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Input.hh>
#include <Fsa/Static.hh>

#include "Application.hh"
#include "BeforeScorer.hh"
#include "ChartFactory.hh"
#include "Parameter.hh"
#include "Permutation.hh"

APPLICATION

using namespace Permute;

class Decode : public Permute::Application {
private:
  static Core::ParameterBool paramDebug;
  bool DEBUG;
public:
  Decode () : Permute::Application ("decode") {}

  virtual void getParameters () {
    Application::getParameters ();
    DEBUG = paramDebug (config);
  }    

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    // Read in the parameters.
    ParameterVector pv;
    if (! this -> parameters (pv)) {
      std::cerr << "Could not read LOP parameter file: " << LOP_FILE << std::endl;
      return EXIT_FAILURE;
    }

    Fsa::ConstAlphabetRef alphabet;
    ChartFactoryRef factory;

    if (LMODEL_FILE != "") {
      // Read in the language model.
      Fsa::StaticAutomaton * lmp = new Fsa::StaticAutomaton;
      Fsa::read (lmp, LMODEL_FILE);
      Fsa::ConstAutomatonRef lm (lmp);

      alphabet = lm -> getInputAlphabet ();
      factory = ChartFactory::outside (lm);
    } else {
      alphabet = Fsa::readAlphabet (ALPHABET_FILE);
      factory = ChartFactory::create ();
    }

    Permutation source (alphabet),
      target (alphabet),
      pos (pv.getPOS ());

    ParseControllerRef controller = this -> parseController (source);

    FeatureCounter counter (pv, pos);

    std::istream & input = this -> input ();
    std::ostream & output = this -> output ();

    for (int i = 1; readPermutation (source, input); ++ i) {
      readPermutation (pos, input);

      target = source;
      readAlignment (target, input);

      ScorerRef scorer = this -> beforeScorer (source, pv, pos);

      ChartRef chart = factory -> chart (source, WINDOW);

      double best_score = scorer -> score (source);
      do {
	Chart::permute (chart, controller, scorer);
	ConstPathRef bestPath = chart -> getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
	if (DEBUG) {
	  std::cerr << source << " (" << bestPath -> getScore () << ")" << std::endl;
	}
      } while (ITERATE_SEARCH && source.changed ());

      output << source << std::endl;
    }
    
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool Decode::paramDebug ("debug", "output intermediate permutations", false);
