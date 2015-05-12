#include <Core/CompressedStream.hh>

#include <Fsa/AlphabetXml.hh>
#include <Fsa/Application.hh>
#include <Fsa/Output.hh>

#include "TTable.hh"

APPLICATION

// Reads an alphabet, a permutation (sentence) and a TTable file.  Constructs
// the appropriate TTable automaton and writes it to a file.
class TestTTable : public Fsa::Application {
private:
  static Core::ParameterString paramAlphabet,
    paramSentence,
    paramTTable,
    paramFsa;
public:
  TestTTable () {
    setTitle ("test-ttable");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }

  int main (const std::vector <std::string> & args) {
    class FiveWeights : public Permute::WeightModel {
    public:
      virtual Fsa::Weight weight (const std::string & line) const {
	std::istringstream in (line);
	double weight = 0.0;
	double f;
	while (in >> f) {
	  weight += 0.2 * f;
	}
	return Fsa::Weight (- ::log (weight));
      }
    } model;

    Fsa::ConstAlphabetRef alphabet = Fsa::readAlphabet (paramAlphabet (config));
    Permute::Permutation p (alphabet);
    std::istringstream in (paramSentence (config));
    Permute::readPermutation (p, in);

    Core::CompressedInputStream ttable (paramTTable (config));

    Fsa::ConstAutomatonRef fsa = Permute::ttable (p, model, ttable);

    Fsa::write (fsa, paramFsa (config));

    return EXIT_SUCCESS;
  }

} app;

Core::ParameterString TestTTable::paramAlphabet ("alphabet", "the alphabet file"),
  TestTTable::paramSentence ("sentence", "the input sentence"),
  TestTTable::paramTTable ("table", "the translation table file"),
  TestTTable::paramFsa ("fsa", "the fsa output file");
