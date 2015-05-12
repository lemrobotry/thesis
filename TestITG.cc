#include <Core/CompressedStream.hh>
#include <Core/Utility.hh>
#include <Fsa/Application.hh>
#include <Fsa/Static.hh>

#include "Chart.hh"
#include "ChartFactory.hh"
#include "Permutation.hh"

APPLICATION

using namespace Permute;

// @bug Superseded by Permute::readAlignment in Permutation.hh.
void readAlignment (Permutation & p, std::istream & in) {
  for (Permutation::iterator i = p.begin (); i != p.end (); ++ i) {
    in >> (* i);
  }
}

// Reads each (source, POS, alignment) example from input and constructs an
// automaton representing the target permutation.  Attempts to parse the source
// using the target automaton and prints (#, length, TRUE/FALSE) depending on
// whether parsing is successful.  This is equivalent to asking whether the
// target permutation is in the ITG neighborhood of the source.
//
// @bug Because a symbol may appear multiple times in the permutation, this
// reports TRUE for some examples even if the actual alignment given is not
// in the ITG neighborhood.
class TestITG : public Fsa::Application {
private:
  static Core::ParameterString paramInput;
public:
  TestITG () {
    setTitle ("test-itg");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }

  bool getline (std::istream & in, std::string & line) {
    return Core::wsgetline (in, line) != EOF;
  }

  int main (const std::vector <std::string> & args) {
    Core::CompressedInputStream input;
    std::istream * in = & std::cin;
    if (paramInput (config) != "-") {
      input.open (paramInput (config));
      in = & input;
    }

    Fsa::StaticAutomaton * targetFsa = new Fsa::StaticAutomaton;
    targetFsa -> setSemiring (Fsa::LogSemiring);
    Fsa::ConstAutomatonRef target (targetFsa);

    ChartFactoryRef factory = ChartFactory::create (target);
    ParseControllerRef pc = CubicParseController::create ();
    ScorerRef scorer (new Scorer);

    std::string line;
    for (int i = 1; getline (* in, line); ++ i) {
      Fsa::copy (targetFsa, line);
      
      std::istringstream iline (line);
      Permutation source (target -> getInputAlphabet ());
      readPermutation (source, iline);
      
      getline (* in, line); // Ignore POS line.
      
      readAlignment (source, * in);

      ChartRef chart = factory -> chart (source);

      Chart::permute (chart, pc, scorer);
      double score = chart -> getBestPath () -> getScore ();

      std::cout << i << '\t' << source.size () << '\t';
      if (score < -100.0) {
	std::cout << "FALSE" << std::endl;
      } else {
	std::cout << "TRUE" << std::endl;
      }
    }
    
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString TestITG::paramInput ("input", "the input sentence source", "-");
