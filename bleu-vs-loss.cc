#include "AdjacentLoss.hh"
#include "Application.hh"
#include "BleuScore.hh"
#include "ChartFactory.hh"
#include "kBestChart.hh"

APPLICATION

using namespace Permute;

class BleuVsLoss : public Application {
private:
  static Core::ParameterBool paramBest;
  bool BEST;
  static Core::ParameterInt paramSamples,
    paramLength;
  int SAMPLES,
    LENGTH;
public:
  BleuVsLoss () :
    Application ("bleu-vs-loss")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    BEST = paramBest (config);
    SAMPLES = paramSamples (config);
    LENGTH = paramLength (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramBest.printShortHelp (out);
    paramSamples.printShortHelp (out);
    paramLength.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation truth;
    integerPermutation (truth, LENGTH);

    Permutation pi (truth);

    ScorerRef scorer (this -> lossScorer (truth, pi));

    AdjacentLoss adj (truth);

    BleuScore bleu;
    bleu.addReferenceLength (1000);
    bleu.addPrecision (1, 1000, 1000);
    bleu.addPrecision (2, 517, 900);
    bleu.addPrecision (3, 307, 800);
    bleu.addPrecision (4, 195, 700);

    std::cerr << "Base BLEU: " << bleu << std::endl;

    if (BEST) {
      ChartFactoryRef factory = ChartFactory::kbest ();
      ParseControllerRef pc = this -> parseController (pi);
      kBestChart kbc (factory -> chart (pi), pc, scorer);
      kbc.permute ();
      kBestChart::Paths paths = kbc.best (SAMPLES);
      for (kBestChart::Paths::const_iterator it = paths.begin ();
	   it != paths.end (); ++ it) {
	pi.reorder (* it);
	std::cout << (bleu + computeBleuScore (truth, pi)).score ()
		  << " "
		  << (* it) -> getScore ()
		  << " "
		  << adj.score (pi)
		  << std::endl;
      }
    } else {
      for (int i = 0; i < SAMPLES; ++ i) {
	pi.randomize ();
	std::cout << (bleu + computeBleuScore (truth, pi)).score ()
		  << " "
		  << scorer -> score (pi)
		  << " "
		  << adj.score (pi)
		  << std::endl;
      }
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool BleuVsLoss::paramBest
("best", "use the k-best permutations instead of random ones", false);
Core::ParameterInt BleuVsLoss::paramSamples
("samples", "the number of random samples", 10, 1);
Core::ParameterInt BleuVsLoss::paramLength
("length", "the length of the permutation", 10, 1);
