#include "Application.hh"
#include "BleuScore.hh"
#include "ChartFactory.hh"
#include "InputData.hh"
#include "LOPChart.hh"
#include "PV.hh"
#include "PrintTimer.hh"

APPLICATION

using namespace Permute;

class SmoothedBleu :
  public std::unary_function <const ConstPathRef &, double>
{
private:
  const BleuScoreAgainst & bleu_;
  Permutation & pi_;
public:
  SmoothedBleu (const BleuScoreAgainst & bleu,
		Permutation & pi) :
    bleu_ (bleu),
    pi_ (pi)
  {}
  double operator () (const ConstPathRef & path) {
    pi_.reorder (path);
    return bleu_.compute (pi_).smoothed ();
  }
};

/**********************************************************************/

class LocalUpdate : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
  static Core::ParameterInt paramK;
  int K;

  PrintTimer timer_;
public:
  LocalUpdate () :
    Application ("local-update-pv"),
    timer_ (std::cerr)
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramK.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    K = paramK (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    timer_.start ("Reading PV: ", LOP_FILE);
    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }
    timer_.stop ("Reading PV: ", LOP_FILE);

    std::vector <WRef> weights (pv.size ());
    std::transform (pv.begin (), pv.end (),
		    weights.begin (),
		    std::select2nd <PV::value_type> ());

    std::vector <double> weightSum (pv.size (), 0.0),
      values (pv.size ());

    double N = 0.0;
    int iteration = 0;

    InputData data (DEPENDENCY);

    ParseControllerRef controller = this -> parseController (data.source ());

    do {
      std::istream & input = this -> input ();

      timer_.start ("Processing Data");
      for (int sentence = 0; sentence < SENTENCES && input >> data; ++ sentence) {
// 	timer_.start ("Sentence ", sentence);
	SumBeforeCostRef bc (new SumBeforeCost (data.source ().size (), "LocalUpdate"));
	ScorerRef scorer = this -> sumBeforeScorer (bc, pv, data);

	LOPkBestChart kbc (data.source ());

// 	timer_.start ("Best Paths: ", K);
	kbc.permute (controller, scorer);
	std::vector <ConstPathRef> paths;
	kbc.getBestPaths (paths, K);
// 	timer_.stop ("Best Paths: ", K);

// 	timer_.start ("BLEU Score: ", paths.size ());
	BleuScoreAgainst bleu (data.target ());
	SmoothedBleu smoothedBleu (bleu, data.source ());

	std::vector <double> smoothed (paths.size ());
	std::transform (paths.begin (), paths.end (),
			smoothed.begin (),
			smoothedBleu);
// 	timer_.stop ("BLEU Score: ", paths.size ());

// 	timer_.start ("Max");
	std::vector <double>::const_iterator it =
	  std::max_element (smoothed.begin (), smoothed.end ());
// 	timer_.stop ("Max");
// 	timer_.start ("Update");
	if (it != smoothed.begin ()) {
	  data.source ().reorder (paths [it - smoothed.begin ()]);
	  this -> perceptronUpdate (bc, data.source (), LEARNING_RATE);
	  data.source ().reorder (paths [0]);
	  this -> perceptronUpdate (bc, data.source (), - LEARNING_RATE);
	}
// 	timer_.stop ("Update");

	update (weightSum, weights);
	++ N;
// 	timer_.stop ("Sentence ", sentence);
      }
      timer_.stop ("Processing Data");

      // Store current weights in values.
      std::copy (weights.begin (), weights.end (),
		 values.begin ());
      std::transform (weightSum.begin (), weightSum.end (),
		      weights.begin (),
		      std::bind2nd (std::divides <double> (), N));
      timer_.start ("Decode Dev");
      this -> decodeDev (pv);
      timer_.stop ("Decode Dev");
      timer_.start ("Write PV");
      this -> writePV (pv, ++ iteration);
      timer_.stop ("Write PV");
      std::copy (values.begin (), values.end (),
		 weights.begin ());
    } while (! this -> converged ());

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat LocalUpdate::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::ParameterInt LocalUpdate::paramK ("k", "the size of the k-best list", 1, 1);
