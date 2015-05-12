#include "Application.hh"
#include "BeforeScorer.hh"
#include "LOPChart.hh"
#include "ParseController.hh"

APPLICATION

using namespace Permute;

// Prints out the k-best permutations in the neighborhood of the identity
// permutation according to a LOLIB matrix.
class QuadratickBestLOP : public Application {
private:
  static Core::ParameterInt paramK;
  int K;
public:
  QuadratickBestLOP () :
    Application ("q-k-best-lop")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    K = paramK (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    BeforeCostRef bcr (this -> lolib ());
    Permutation p;
    integerPermutation (p, bcr -> size ());

    ParseControllerRef pc = this -> parseController (p);
    ScorerRef gamma (new BeforeScorer (bcr, p));

    QNormLOPkBestChart chart (p, QUADRATIC_WIDTH);
    chart.permute (pc, gamma);

    std::vector <ConstPathRef> best;
    chart.getBestPaths (best, K);

    for (std::vector <ConstPathRef>::const_iterator it = best.begin ();
	 it != best.end (); ++ it) {
      p.reorder (* it);
      std::cout << p << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt QuadratickBestLOP::paramK ("k", "the number of best paths", 2, 1);
