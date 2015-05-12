#include "Application.hh"
#include "ChartFactory.hh"

APPLICATION

using namespace Permute;

// Starts at each permutation of length n and counts the number of steps k
// search requires to reach the identity permutation.  Writes out any for which
// k is at least K.
class PathLength : public Application {
private:
  static Core::ParameterInt paramN, paramK;
  int N, K;
public:
  PathLength () :
    Application ("path-length")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    N = paramN (config);
    K = paramK (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    std::ostream & out = this -> output ();

    Permutation pi, source;
    integerPermutation (pi, N);
    source = pi;

    ScorerRef loss = this -> lossScorer (source, pi);
    ChartFactoryRef factory = ChartFactory::create ();
    ParseControllerRef controller = this -> parseController (source);

    // next_permutation returns false when it arrives back at identity.
    while (std::next_permutation (pi.begin (), pi.end ())) {
      source = pi;
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

      if (length >= K) {
	out << "Length = " << length
	    << " Permutation = " << pi
	    << std::endl;
      }
    }
  }
} app;

// Four is the minimum interesting case, so it is the default.
Core::ParameterInt PathLength::paramN ("n", "the permutation length", 4, 1);
Core::ParameterInt PathLength::paramK ("k", "the path length of interest", 3, 1);
