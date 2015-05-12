#include "Application.hh"
#include "Parameter.hh"

APPLICATION

using namespace Permute;

class TrimParameters : public Application {
private:
  static Core::ParameterFloat paramThreshold;
public:
  TrimParameters () : Application ("trim-parameters") {}

  int main (const std::vector <std::string> & args) {
    ParameterVector pv;
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }
    double threshold = paramThreshold (config);

    std::cerr << "Considering " << pv.size () << " parameters" << std::endl;

    for (ParameterVector::parameter_iterator p = pv.begin_p (); p != pv.end_p (); ++ p) {
      if (p -> second < threshold) {
	pv.erase (p);
      } else {
	p -> second = 0.0;
      }
    }

    std::cerr << "Kept " << pv.size () << " parameters" << std::endl;

    this -> outputParameters (pv);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat TrimParameters::paramThreshold ("threshold", "the smallest count to allow", 2.0, 0.0);
