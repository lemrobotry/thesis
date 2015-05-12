#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

typedef std::pair <std::string, WRef> Param;

class LessFeature :
  std::binary_function <const Param &, const Param &, bool>
{
public:
  bool operator () (const Param & a,
		    const Param & b) const {
    return static_cast <double> (a.second) < static_cast <double> (b.second);
  }
};

class LessAbsoluteFeature :
  std::binary_function <const Param &, const Param &, bool>
{
public:
  bool operator () (const Param & a,
		    const Param & b) const {
    return std::fabs (static_cast <double> (a.second))
      < std::fabs (static_cast <double> (b.second));
  }
};

class PrintFeature :
  std::unary_function <const Param &, void>
{
private:
  const PV & pv_;
public:
  PrintFeature (const PV & pv) : pv_ (pv) {}
  void operator () (const Param & feature) const {
    std::cout << static_cast <double> (feature.second)
	      << " "
	      << pv_.uncompress (feature.first)
	      << std::endl;
  }
};

class ExtremeFeatures : public Application {
private:
  static Core::ParameterInt paramBestN,
    paramWorstN,
    paramMiddleN;
  int BEST_N, WORST_N, MIDDLE_N;
public:
  ExtremeFeatures () :
    Application ("extreme-features-pv")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    BEST_N = paramBestN (config);
    WORST_N = paramWorstN (config);
    MIDDLE_N = paramMiddleN (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramBestN.printShortHelp (out);
    paramWorstN.printShortHelp (out);
    paramMiddleN.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <Param> features (pv.begin (), pv.end ());

    std::sort (features.begin (), features.end (), LessFeature ());

    PrintFeature print_feature (pv);

    if (BEST_N) {
      std::cout << "Best:" << std::endl;
      std::for_each (features.rbegin (), features.rbegin () + BEST_N, print_feature);
    }
    if (WORST_N) {
      std::cout << std::endl << "Worst:" << std::endl;
      std::for_each (features.begin (), features.begin () + WORST_N, print_feature);
    }

    if (MIDDLE_N) {
      std::sort (features.begin (), features.end (), LessAbsoluteFeature ());
      std::cout << std::endl << "Middle:" << std::endl;
      std::for_each (features.begin (), features.begin () + MIDDLE_N, print_feature);
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt ExtremeFeatures::paramBestN ("best", "the number of best features to output", 50, 0);
Core::ParameterInt ExtremeFeatures::paramWorstN ("worst", "the number of worst features to output", 50, 0);
Core::ParameterInt ExtremeFeatures::paramMiddleN ("middle", "the number of middle features to output", 50, 0);
