#include <Core/TextStream.hh>
#include <Fsa/Application.hh>
#include <Fsa/Output.hh>

#include "nGram.hh"

APPLICATION

// Wraps the function ngram from nGram.hh in an application.
class MakeNGram : public Fsa::Application {
private:
  static Core::ParameterString paramInputFile;
  static Core::ParameterString paramOutputFile;
public:
  MakeNGram () {
    setTitle ("make-ngram");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }
  int main (const std::vector <std::string> & args) {
    Fsa::ConstAutomatonRef ngram =
      Permute::ngram (paramInputFile (config) == "-" ? std::cin :
		      Core::TextInputStream (paramInputFile (config)));
    Fsa::write (ngram, paramOutputFile (config));
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString MakeNGram::paramInputFile ("input", "the count input file", "-");
Core::ParameterString MakeNGram::paramOutputFile ("output", "the model output file", "-");
