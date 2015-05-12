#include "Application.hh"
#include "ChartFactory.hh"

APPLICATION

using namespace Permute;

// Measures the number of steps required to reach the target permutation from
// the source permutation using the ITG neighborhood and greedy search with the
// loss scorer.
class GreedyPathLength : public Application {
public:
  GreedyPathLength () :
    Application ("greedy-path-length")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();

    Permutation source, target, pos;

    ChartFactoryRef factory = ChartFactory::create ();

    for (int sentence = 1;
	 sentence <= SENTENCES &&
	   readPermutationWithAlphabet (source, in);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, in);
      target = source;
      readAlignment (target, in);

      ScorerRef loss = this -> lossScorer (source, target);
      ParseControllerRef controller = this -> parseController (source);

      int length = 0;
      ChartRef chart = factory -> chart (source, WINDOW);
      double best_score = loss -> score (source);
      do {
	source.changed (false);
	Chart::permute (chart, controller, loss);
	ConstPathRef best_path = chart -> getBestPath ();
	if (best_path -> getScore () > best_score) {
	  best_score = best_path -> getScore ();
	  source.reorder (best_path);
	  ++ length;
	}
      } while (source.changed ());

      out << sentence << '\t'
	  << source.size () << '\t'
	  << length << std::endl; 

    }

    return EXIT_SUCCESS;
  }
} app;
