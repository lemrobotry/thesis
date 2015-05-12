#include "Application.hh"
#include "BeforeScorer.hh"
#include "LOPChart.hh"
#include "ParseController.hh"

APPLICATION

using namespace Permute;

class QuadraticLOP : public Application {
private:
  static Core::ParameterInt paramRestart;
  int RESTART;
  Core::Timer timer;
public:
  QuadraticLOP () :
    Application ("q-lop")
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
      
      Permutation p;
      integerPermutation (p, bcr -> size ());

      ParseControllerRef pc = this -> parseController (p);
      ScorerRef gamma (new BeforeScorer (bcr, p));

      QuadraticNormalLOPChart chart (p, QUADRATIC_WIDTH, QUADRATIC_LEFT > 0);

      double best_score = Core::Type <double>::min;

      for (int i = 0; i < RESTART; ++ i) {
	timer.start ();
	
	p.randomize ();
	best_score = gamma -> score (p);
	int iterations = 0;
	do {
	  ++ iterations;
	  chart.permute (pc, gamma);
	  const ConstPathRef & bestPath = chart.getBestPath ();
	  p.changed (false);
	  if (bestPath -> getScore () > best_score) {
	    best_score = bestPath -> getScore ();
	    p.reorder (bestPath);
	  }
	} while (p.changed ());

	timer.stop ();

	std::cout << index ++ << ' '
		  << bcr -> name () << ' '
		  << best_score << ' '
		  << iterations << ' '
		  << timer.user () << std::endl;
      }
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt QuadraticLOP::paramRestart ("restart", "the number of restarts", 1, 1);
