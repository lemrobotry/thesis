#include <Core/Parameter.hh>
#include <Core/Statistics.hh>
#include <Fsa/Application.hh>
#include <Fsa/Best.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Input.hh>
#include <Fsa/Output.hh>
#include <Fsa/Permute.hh>
#include <Fsa/Project.hh>
#include <Fsa/Prune.hh>
#include <Fsa/Static.hh>

APPLICATION

// Uses Fsa::permute and Fsa::pruneSync to decode sentences read from input
// using a translation automaton passed as the --fsa command-line argument.
// Fsa::permute creates an IBMPermuteAutomaton, which allows all possible
// reorderings when no window size or distortion limit are given.
// Fsa::pruneSync uses "slices" equivalent to stacks, I think, taking epsilon
// transitions into account, and prunes to a threshold (as opposed to a fixed
// size).
class BeamSearch : public Fsa::Application {
private:
  static Core::ParameterString paramFsa;
  static Core::ParameterFloat paramThreshold;
  Core::Timer timer;
public:
  BeamSearch () {
    setTitle ("beam-search");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }
  Fsa::ConstAutomatonRef read (const std::string & file) {
    std::cerr << "read: " << file << std::endl;
    Fsa::StaticAutomaton * fst = new Fsa::StaticAutomaton;
    Fsa::read (fst, file);
    return Fsa::ConstAutomatonRef (fst);
  }
  Fsa::ConstAutomatonRef readLinear (std::istream & in, Fsa::ConstSemiringRef semiring) {
    Fsa::StaticAutomaton * linear = new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    linear -> setSemiring (semiring);
    Fsa::readLinear (linear, in);
    return Fsa::ConstAutomatonRef (linear);
  }
  int main (const std::vector <std::string> & args) {
    Fsa::ConstAutomatonRef fsa = read (paramFsa (config));
    Fsa::ConstAutomatonRef sentence;

    std::cerr << "** Threshold: " << paramThreshold (config) << " **" << std::endl;

    while (std::cin) {
      sentence = readLinear (std::cin, fsa -> semiring ());
      timer.start ();
      Fsa::ConstAutomatonRef best =
	Fsa::best (Fsa::pruneSync (Fsa::composeMatching (Fsa::permute (sentence), fsa, false),
				   Fsa::Weight (paramThreshold (config))));
      std::cout << float (Fsa::bestscore (best)) << ": ";
      Fsa::writeLinear (Fsa::projectInput (best), std::cout);
      std::cout << std::endl;
      timer.stop ();
      std::cout << timer.user () << "u " << timer.system () << "s " << timer.elapsed () << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString BeamSearch::paramFsa ("fsa", "the scoring automaton", "");
Core::ParameterFloat BeamSearch::paramThreshold ("threshold", "the pruning threshold", 4.605170185988091);
