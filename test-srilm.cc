#include <Core/CompressedStream.hh>

#include <Fsa/Application.hh>
#include <Fsa/Input.hh>
#include <Fsa/Output.hh>

#include "SRILM.hh"

APPLICATION

// Uses Permute::srilm (see SRILM.hh) to generate a language model automaton
// containing the subset of the language model relevant to the output of the
// transducer given as the --ttable command-line argument.
class TestSRILM : public Fsa::Application {
private:
  static Core::ParameterString paramTTable,
    paramLM,
    paramFsa;
public:
  TestSRILM () {
    setTitle ("test-srilm");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }

  int main (const std::vector <std::string> & args) {
    Fsa::ConstAutomatonRef ttable = Fsa::read (paramTTable (config), Fsa::LogSemiring);

    Core::CompressedInputStream input (paramLM (config));

    Fsa::ConstAutomatonRef lm = Permute::srilm (ttable -> getOutputAlphabet (), input);

    Fsa::write (lm, paramFsa (config));

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString TestSRILM::paramTTable ("ttable", "t-table fsa file"),
  TestSRILM::paramLM ("lm", "SRI LM file"),
  TestSRILM::paramFsa ("fsa", "output fsa file");
