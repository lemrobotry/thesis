#include <Fsa/Best.hh>
#include <Fsa/Cache.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Output.hh>
#include <Fsa/Project.hh>

#include "Application.hh"
#include "FsaCellMap.hh"
#include "Permutation.hh"
#include "PruneChart.hh"

APPLICATION

// Contructs a distortion model, translation model, and language model
// appropriate for each sentence in the given input, and uses permutation search
// to find the best reordering of the source sentence to pass through the
// composition of those models to get the target sentence.
class Translate : public Permute::Application {
private:
  static Core::ParameterBool paramDebug;
  bool DEBUG;
  static Core::ParameterFloat paramBeamThreshold;
  double BEAM_THRESHOLD;
public:
  Translate () : Permute::Application ("translate") {}

  virtual void getParameters () {
    Application::getParameters ();
    DEBUG = paramDebug (config);
    BEAM_THRESHOLD = paramBeamThreshold (config);
  }

  int main (const std::vector <std::string> & args) {

    Permute::Permutation p;

    Permute::ParseControllerRef controller = this -> parseController (p);
    Permute::ScorerRef scorer = this -> scorer (); 

    std::ostream & target = this -> output ();
    std::istream & source = this -> input ();
    while (readPermutationWithAlphabet (p, source)) {
      Fsa::ConstAutomatonRef sentence = Permute::fsa (p, Fsa::LogSemiring);

      Fsa::ConstAutomatonRef distortion = this -> distortion (p);
      Fsa::ConstAutomatonRef channel = this -> ttable (p);
      Fsa::ConstAutomatonRef language = this -> lmodel (channel -> getOutputAlphabet ());

      if (DEBUG) {
	Fsa::write (sentence, "translate.debug.source.gz");
	Fsa::write (channel, "translate.debug.ttable.gz");
	Fsa::write (language, "translate.debug.srilm.gz");
      }

      Fsa::ConstAutomatonRef a =
	Fsa::cache (Fsa::composeMatching (distortion,
					  Fsa::composeMatching (channel,
								language,
								false),
					  false));
      
      // Workaround since outside pruning doesn't happen on the first
      // iteration using the ChartFactory.
      std::pair <Permute::CellMapRef, Permute::CellRef> mapPair = Permute::FsaCellMap::mapFsa (a);
      Permute::ChartRef inside (new Permute::ChartImpl (p, Permute::CellMap::cache (mapPair.first), mapPair.second));
      std::pair <Permute::CellMapRef, Permute::CellRef> oMapPair = Permute::UnigramFsaCellMap::mapFsa (a);
      Permute::ChartRef outside (new Permute::ChartImpl (p, Permute::CellMap::cache (oMapPair.first), oMapPair.second));
      Fsa::Weight threshold = Fsa::bestscore (Fsa::composeMatching (sentence, a));
      Permute::ChartRef chart = Permute::prune (inside, outside, threshold);
//       chart = Permute::pruneRelative (chart, Fsa::Weight (::log (BEAM_THRESHOLD)));

      double best_score = scorer -> score (source);
      do {
	Permute::Chart::permute (chart, controller, scorer);
	Permute::ConstPathRef bestPath = chart -> getBestPath ();
	p.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  p.reorder (bestPath);
	}
	std::cerr << p << std::endl;
      } while (p.changed ());

      sentence = Permute::fsa (p, Fsa::LogSemiring);
      Fsa::ConstAutomatonRef best =
	Fsa::projectOutput (Fsa::best (Fsa::composeMatching (sentence, a)));
      Fsa::writeLinear (best, target);
      target << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool Translate::paramDebug ("debug", "output automaton files", false);
Core::ParameterFloat Translate::paramBeamThreshold ("beam-threshold", "the minimum value of a hypothesis relative to the best hypothesis in a cell", 0.00001, 0.0, 1.0);
