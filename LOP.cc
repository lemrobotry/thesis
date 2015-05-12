/* Roy W. Tromble
   18 September 2006
*/

#include "Application.hh"
#include "BeforeScorer.hh"
#include "LOPChart.hh"
#include "ParseController.hh"

// Stores user, system, and elapsed time, and accumulates counts from
// Core::Timer.
class TotalTime {
private:
  double user, system, elapsed;
public:
  TotalTime () :
    user (0.0),
    system (0.0),
    elapsed (0.0)
  {}
  const TotalTime & operator += (const Core::Timer & timer) {
    user += timer.user ();
    system += timer.system ();
    elapsed += timer.elapsed ();
  }
  void write (Core::XmlWriter & os) const {
    os << Core::XmlOpen ("total-time")
       << Core::XmlFull ("user", user)
       << Core::XmlFull ("system", system)
       << Core::XmlFull ("elapsed", elapsed)
       << Core::XmlClose ("total-time");
  }
  friend Core::XmlWriter & operator << (Core::XmlWriter & os, const TotalTime & time);
};

Core::XmlWriter & operator << (Core::XmlWriter & os, const TotalTime & time) {
  time.write (os);
  return os;
}

APPLICATION

using namespace Permute;

// Reads a LOLIB format LOP cost matrix from the --lolib-file parameter.
// Searches for the best permutation using the VLSN and prints it.  Uses a
// random starting point, restarts, and pruning on request.
class LOP : public Application {
private:
  static Core::ParameterBool paramRandom;
  static Core::ParameterInt paramRestart;
  static Core::ParameterBool paramPrune;
  bool RANDOM, PRUNE;
  int RESTART;
  Core::Timer timer;
  Core::XmlWriter out;
public:
  LOP () :
    Application ("LOP"),
    out (std::cerr)
  {}

  virtual void getParameters () {
    Application::getParameters ();
    RANDOM = paramRandom (config);
    RESTART = paramRestart (config);
    PRUNE = paramPrune (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::cout << "Name Score Iterations Time" << std::endl;
    int index = 0;
    for (std::vector <std::string>::const_iterator it = args.begin (); it != args.end (); ++ it) {
      this -> LOLIB_FILE = * it;
      BeforeCost * bc = this -> lolib ();
      double excess = bc -> normalize ();
      BeforeCostRef bcr (bc);

      std::stringstream str;
      for (int i = 0; i < bcr -> size (); ++ i) {
	str << i << ' ';
      }
      Permutation p;
      readPermutationWithAlphabet (p, str);

      ParseControllerRef pc = this -> parseController (p);

      LOPChart chart (p, WINDOW);

      ScorerRef gamma (new BeforeScorer (bcr, p));

      double best_score = Core::Type <double>::min;

      for (int i = 0; i < RESTART; ++ i) {
	timer.start ();
	if (RANDOM) {
	  p.randomize ();
	}

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
		  << excess + best_score << ' '
		  << iterations << ' '
		  << timer.user () << std::endl;
      }
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool LOP::paramRandom ("random", "start at a random permutation?", false);
Core::ParameterInt LOP::paramRestart ("restart", "the number of restarts", 1, 1);
Core::ParameterBool LOP::paramPrune ("prune", "prune the chart?", false);
