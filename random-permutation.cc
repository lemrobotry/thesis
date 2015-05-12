// Roy W. Tromble
// 2 November 2007

#include "Application.hh"

APPLICATION

using namespace Permute;

// Reads a LOLIB format LOP cost matrix from the --lolib parameter.  Generates a
// sequence of random permutations and reports their score, along with the time
// required to compute both.
class RandomPermutation : public Application {
private:
  static Core::ParameterInt paramRestart;
  int RESTART;
public:
  RandomPermutation () :
    Application ("random-permutation")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    RESTART = paramRestart (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    std::cout << "Name Score Iterations Time" << std::endl;
    int index = 0;
    for (std::vector <std::string>::const_iterator it = args.begin (); it != args.end (); ++ it) {
      this -> LOLIB_FILE = * it;
      BeforeCostRef bcr (this -> lolib ());
      std::stringstream str;
      for (int i = 0; i < bcr -> size (); ++ i) {
	str << i << ' ';
      }
      Permutation pi;
      readPermutationWithAlphabet (pi, str);
      ScorerRef scorer (new BeforeScorer (bcr, pi));

      for (int i = 0; i < RESTART; ++ i) {
	Core::Timer timer;
	timer.start ();
	pi.randomize ();
	double score = scorer -> score (pi);
	timer.stop ();

	std::cout << index ++ << ' '
		  << bcr -> name () << ' '
		  << score << ' '
		  << "1 "
		  << timer.user () << std::endl;
      }
    }
  }
} app;

Core::ParameterInt RandomPermutation::paramRestart ("restart", "the number of restarts", 1, 1);
