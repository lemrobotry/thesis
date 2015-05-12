#include <Fsa/Best.hh>
#include <Fsa/Cache.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Linear.hh>
#include <Fsa/Output.hh>
#include <Fsa/Project.hh>

#include "Application.hh"
#include "Fsa.hh"
#include "InputData.hh"
#include "Iterator.hh"
#include "LOPChart.hh"
#include "PV.hh"
#include "PrintTimer.hh"

APPLICATION

using namespace Permute;

class TranslatePath :
  public std::unary_function <const ConstPathRef &, std::pair <std::string, double> >
{
private:
  BestOutput bestOutput_;
  Permutation & pi_;
  Fsa::ConstAutomatonRef a_;
public:
  TranslatePath (Permutation & pi, Fsa::ConstAutomatonRef a) :
    bestOutput_ (),
    pi_ (pi),
    a_ (a)
  {}
  std::pair <std::string, double> operator () (const ConstPathRef & path) const {
    pi_.reorder (path);
    Fsa::ConstAutomatonRef sentence = fsa (pi_, Fsa::TropicalSemiring),
      best = bestOutput_ (sentence, a_);
    std::ostringstream out;
    Fsa::writeLinear (best, out);
    return std::make_pair (out.str (), path -> getScore () - static_cast <double> (Fsa::getLinearWeight (best))); 
  }
};

class kBestTranslate : public Application {
private:
  static Core::ParameterInt paramK;
  int K;
  PrintTimer timer_;
public:
  kBestTranslate () :
    Application ("k-best-translate"),
    timer_ (std::cerr)
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramK.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    timer_.start ("Reading PV: ", LOP_FILE);
    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }
    timer_.stop ("Reading PV: ", LOP_FILE);

    InputData line (DEPENDENCY);

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();

    timer_.start ("Reading LM: ", LMODEL_FILE);
    Fsa::ConstAutomatonRef language = this -> lmodel ();
    timer_.stop ("Reading LM: ", LMODEL_FILE);

    ParseControllerRef controller = this -> parseController (line.source ());

    while (in >> line) {
      SumBeforeCostRef bc (new SumBeforeCost (line.source ().size (), "kBestTranslate"));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, line);

      LOPkBestChart kbc (line.source ());

      kbc.permute (controller, scorer);
      std::vector <ConstPathRef> paths;
      kbc.getBestPaths (paths, K);

      Fsa::ConstAutomatonRef channel = this -> ttable (line.source (), language -> getInputAlphabet ()),
	a = Fsa::cache (Fsa::composeMatching (channel, language, false));

      TranslatePath translator (line.source (), a);

      std::vector <std::pair <std::string, double> > outputs (paths.size ());
      std::transform (paths.begin (), paths.end (),
		      outputs.begin (),
		      translator);

      std::vector <std::pair <std::string, double> >::const_iterator it =
	std::max_element (outputs.begin (), outputs.end (),
			  compose_bu (std::less <double> (),
				      std::select2nd <std::pair <std::string, double> > ()));

      out << it -> first << std::endl;
    }

    return EXIT_SUCCESS;
  }

} app;

Core::ParameterInt kBestTranslate::paramK ("k", "the size of the k-best list", 1, 1);
