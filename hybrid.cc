#include "Application.hh"
#include "BeforeScorer.hh"
#include "LinearOrdering.hh"
#include "LOPChart.hh"

APPLICATION

using namespace Permute;

// Runs some combination of LS_f, Q(2), and Cubic neighborhood local search,
// starting with the most efficient method specified and invoking the less
// efficient, larger, neighborhoods only when the more efficient ones get
// stuck.  If --iterate is true, runs only a single step of Q(2) or Cubic when
// necessary and starts again on LS_f.
class Hybrid : public Application {
private:
  static Core::ParameterBool paramLSf, paramQ2, paramCubic, paramIterate;
  bool LSF, Q2, CUBIC, ITERATE;
  static Core::ParameterInt paramRestart;
  int RESTART;
  Core::Timer timer;
public:
  Hybrid () :
    Application ("hybrid")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    LSF = paramLSf (config);
    Q2 = paramQ2 (config);
    CUBIC = paramCubic (config);
    ITERATE = paramIterate (config);
    RESTART = paramRestart (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    ParseControllerRef qpc = QuadraticParseController::create (2);
    ParseControllerRef cpc = CubicParseController::create ();
    
    std::cout << "Name Score Iterations Time" << std::endl;
    int index = 0;
    for (std::vector <std::string>::const_iterator it = args.begin (); it != args.end (); ++ it) {
      this -> LOLIB_FILE = * it;
      // Reads the LOLIB cost matrix.
      BeforeCostRef bcr (this -> lolib ());
      // Constructs the permutation.
      Permutation pi;
      integerPermutation (pi, bcr -> size ());

      ScorerRef gamma (new BeforeScorer (bcr, pi));
      LOPChart chart (pi);

      for (int restart = 0; restart < RESTART; ++ restart) {
	timer.start ();
	pi.randomize ();
	// Counts the total number of iterations.
	int iterations = 0;
	double score;
	do {
	  if (LSF) {
	    for (++ iterations; visit (pi, bcr) > 0; ++ iterations);
	  }
	  std::cerr << "LSF: " << iterations << " ";
	  score = gamma -> score (pi);
	  if (Q2) {
	    do {
	      ++ iterations;
	      chart.permute (qpc, gamma);
	      const ConstPathRef & bestPath = chart.getBestPath ();
	      pi.changed (false);
	      if (bestPath -> getScore () > score) {
		score = bestPath -> getScore ();
		pi.reorder (bestPath);
	      }
	    } while (pi.changed () && ! ITERATE);
	    std::cerr << "Q2: " << iterations << " ";
	  }
	  // If Q2 && ITERATE && pi.changed () then cubic doesn't run yet.
	  if (CUBIC && !(Q2 && ITERATE && pi.changed ())) {
	    do {
	      ++ iterations;
	      chart.permute (cpc, gamma);
	      const ConstPathRef & bestPath = chart.getBestPath ();
	      pi.changed (false);
	      if (bestPath -> getScore () > score) {
		score = bestPath -> getScore ();
		pi.reorder (bestPath);
	      }
	    } while (pi.changed () && ! ITERATE);
	    std::cerr << "Cubic: " << iterations << " ";
	  }
	} while (ITERATE && pi.changed ());
	timer.stop ();
	std::cerr << std::endl;
	std::cout << index ++ << ' '
		  << bcr -> name () << ' '
		  << score << ' '
		  << iterations << ' '
		  << timer.user () << std::endl;
      }
    }
    return EXIT_SUCCESS;
  }   
} app;

Core::ParameterBool Hybrid::paramLSf ("lsf", "Use the LS_f local search method", true);
Core::ParameterBool Hybrid::paramQ2 ("q2", "Use the Q(2) quadratic-time VLSN", true);
Core::ParameterBool Hybrid::paramCubic ("cubic", "Use the cubic-time VLSN", true);
Core::ParameterBool Hybrid::paramIterate ("iterate", "Run a single step of Q(2)/Cubic and restart LS_f", true);
Core::ParameterInt Hybrid::paramRestart ("restart", "The number of random restarts", 100, 1);
