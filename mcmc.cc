#include "Application.hh"
#include "LinearOrdering.hh"
#include "MetropolisHastings.hh"

APPLICATION

using namespace Permute;

class MCMC : public Application {
private:
  static Core::ParameterInt paramSamples;
  int SAMPLES;
  static Core::ParameterFloat paramAlpha;
  double ALPHA;
public:
  MCMC () :
    Application ("mcmc")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    SAMPLES = paramSamples (config);
    ALPHA = paramAlpha (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    BeforeCostRef bc (this -> lolib ());

    std::stringstream str;
    for (int i = 0; i < bc -> size (); ++ i) {
      str << i << ' ';
    }
    Permutation pi;
    readPermutationWithAlphabet (pi, str);

    AdjacentTransposition * neighborhood = new AdjacentTransposition (pi, bc, ALPHA);

    MetropolisHastings mh (neighborhood);

    for (int i = 0; i < SAMPLES; ++ i) {
      Permutation sample (pi);
      mh.sample (sample);

      std::cout << sample << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt MCMC::paramSamples ("samples", "the number of samples", 1, 0);
Core::ParameterFloat MCMC::paramAlpha ("alpha", "the scaling factor", 1.0, 0.0);
