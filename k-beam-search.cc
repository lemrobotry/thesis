#include <Core/CompressedStream.hh>
#include <Core/Parameter.hh>
#include <Core/Statistics.hh>
#include <Core/XmlStream.hh>
#include <Fsa/Application.hh>
#include <Fsa/Best.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Input.hh>
#include <Fsa/Output.hh>
#include <Fsa/Permute.hh>
#include <Fsa/Project.hh>
#include <Fsa/Static.hh>

#include "BeamSearch.hh"

APPLICATION

// Performs fixed-width beam search (also called "histogram pruning") to decode
// each sentence in the standard input using the transducer passed as the --fsa
// argument and Fsa::permute to build the lattice of all permutations of the
// input.  See BeamSearch.hh for a description of the search.
class kBeamSearch : public Fsa::Application {
private:
  static Core::ParameterString paramFsa;
  static Core::ParameterString paramOutput;
  static Core::ParameterInt paramWidth;
  Core::Timer timer;
  Core::XmlWriter * out;
public:
  kBeamSearch () {
    setTitle ("k-beam-search");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }
  Fsa::ConstAutomatonRef read (const std::string & file) {
    (* out) << Core::XmlFull ("automaton", file) << "\n";
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
    Core::CompressedOutputStream co (paramOutput (config));
    Core::XmlWriter out (co);
    this -> out = & out;
    out << Core::XmlOpen ("beam-search")
         + Core::XmlAttribute ("width", paramWidth (config))
	<< "\n";

    Fsa::ConstAutomatonRef fsa = read (paramFsa (config));
    Fsa::ConstAutomatonRef sentence;

    for (int i = 1; std::cin; ++ i) {
      sentence = readLinear (std::cin, fsa -> semiring ());
      if (sentence -> getState (sentence -> initialStateId ()) -> isFinal ()) {
	continue;
      }
      out << Core::XmlOpen ("sentence")
	   + Core::XmlAttribute ("number", i)
	  << "\n";
      timer.start ();

      std::pair <Fsa::ConstAutomatonRef, float> bestPair =
	Permute::beamSearch (Fsa::composeMatching (Fsa::permute (sentence), fsa, false),
			     1 + paramWidth (config));
      Fsa::ConstAutomatonRef best = bestPair.first;
      
      timer.stop ();

      out << Core::XmlFull ("score", - float (Fsa::bestscore (best))) << "\n";
      out << Core::XmlFull ("beam-search-score", bestPair.second) << "\n";
      out << Core::XmlOpen ("permutation");
      Fsa::writeLinear (Fsa::projectInput (best), out);
      out << Core::XmlClose ("permutation") << "\n";

      timer.write (out);
      out << "\n";
      
      out << Core::XmlClose ("sentence") << "\n";
    }
    
    out << Core::XmlClose ("beam-search") << "\n";
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString kBeamSearch::paramFsa ("fsa", "the scoring automaton", "");
Core::ParameterString kBeamSearch::paramOutput ("output", "output file", "-");
Core::ParameterInt kBeamSearch::paramWidth ("width", "the beam width", 1);
