#include "Application.hh"
#include "BeforeScorer.hh"
#include "Iterator.hh"
#include "LOPChart.hh"
#include "ParseController.hh"

APPLICATION

using namespace Permute;

class QSize : public Application {
private:
  static Core::ParameterInt paramN;
  int N;
  static Core::ParameterBool paramPrint;
  bool PRINT;
public:
  QSize () :
    Application ("q-size")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    N = paramN (config);
    PRINT = paramPrint (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramN.printShortHelp (out);
    paramPrint.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation pi;
    integerPermutation (pi, N);

    ParseControllerRef pc = this -> parseController (pi);
    ScorerRef scorer = tauScorer (pi, pi);
    
    QNormLOPkBestChart chart (pi, QUADRATIC_WIDTH);
    chart.permute (pc, scorer);

    std::vector <ConstPathRef> best;
    chart.getBestPaths (best, std::numeric_limits <int>::max ());

    std::cerr << best.size () << std::endl;

    if (PRINT) {
      PrintPath printer (pi);
      std::for_each (best.begin (), best.end (), printer);
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt QSize::paramN ("n", "the size of the permutation", 1, 1);
Core::ParameterBool QSize::paramPrint ("print", "print the permutations", false);
