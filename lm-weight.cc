#include <Fsa/Linear.hh>

#include "Application.hh"
#include "Fsa.hh"

APPLICATION

using namespace Permute;

class LMWeight : public Application {
private:
  BestOutput bestOutput_;
  static Core::ParameterString paramMosesFile;
  std::string MOSES_FILE;
public:
  LMWeight () :
    Application ("lm-weight"),
    bestOutput_ ()
  {}

  virtual void getParameters () {
    Application::getParameters ();
    MOSES_FILE = paramMosesFile (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramMosesFile.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    std::istream & in = this -> input ();
    Core::CompressedInputStream mosesIn (MOSES_FILE);

    Fsa::ConstAutomatonRef language = this -> lmodel ();
    std::cerr << "Language Model size: "
	      << language -> getMemoryUsed ()
	      << std::endl
	      << "Alphabet size: "
	      << language -> getInputAlphabet () -> getMemoryUsed ()
	      << std::endl;

    while (readPermutationWithAlphabet (source, in)) {
      readPermutationWithAlphabet (pos, in);
      if (DEPENDENCY) {
	readParents (parents, in);
	readPermutationWithAlphabet (labels, in);
      }
      target = source;
      readAlignment (target, in);

      Fsa::ConstAutomatonRef
	sentence = fsa (source, Fsa::TropicalSemiring),
	channel = this -> ttable (source, language -> getInputAlphabet ()),
	a = Fsa::composeMatching (channel, language, true);

      Permutation moses (language -> getInputAlphabet ());
      readPermutation (moses, mosesIn);

      Fsa::ConstAutomatonRef
	mosesSentence = fsa (moses, Fsa::TropicalSemiring),
	channelOut = Fsa::composeMatching (sentence, channel, true),
	bestTT = bestOutput_ (channelOut, mosesSentence),
	bestLM = bestOutput_ (mosesSentence, language),
	best = bestOutput_ (sentence, a);

      std::cerr << "Channel: " << channelOut -> initialStateId ()
		<< " TT: " << bestTT -> initialStateId ()
		<< " LM: " << bestLM -> initialStateId ()
		<< " A: " << best -> initialStateId ()
		<< std::endl;

      std::cerr << moses << std::endl << "TT: "
		<< static_cast <double> (Fsa::getLinearWeight (bestTT))
		<< std::endl;
      std::cerr << "LM: "
		<< static_cast <double> (Fsa::getLinearWeight (bestLM))
		<< std::endl;
      std::cerr << "A: "
		<< static_cast <double> (Fsa::getLinearWeight (best))
		<< std::endl;
      Fsa::writeLinear (best, std::cerr);
      std::cerr << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString LMWeight::paramMosesFile ("moses", "the moses translation file");
