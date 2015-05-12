#include <Core/CompressedStream.hh>

#include <Fsa/AlphabetXml.hh>
#include <Fsa/Application.hh>

#include "Parameter.hh"
#include "Permutation.hh"

APPLICATION

using namespace Permute;

// Reads a LOP ParameterVector from a file, then reads each (source, POS,
// alignment) triple from input and counts the features that occur in source and
// target permutations.  Prints each feature along with these two counts.
class FeatureCount : public Fsa::Application {
private:
  static Core::ParameterString paramLOP;
  static Core::ParameterString paramAlphabet;
  static Core::ParameterString paramInput;
public:
  FeatureCount () :
    Fsa::Application ()
  {
    setTitle ("feature-count");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }

  int main (const std::vector <std::string> & args) {
    ParameterVector from, to;
    ParameterVectorXmlParser from_parser (config, from), to_parser (config, to);
    if (! from_parser.parseFile (paramLOP (config)) || ! to_parser.parseFile (paramLOP (config))) {
      std::cerr << "Could not read LOP parameter file: " << paramLOP (config) << std::endl;
      return EXIT_FAILURE;
    }

    Fsa::ConstAlphabetRef alphabet = Fsa::readAlphabet (paramAlphabet (config));

    Permutation source (alphabet), target (alphabet), pos (from.getPOS ());

    FeatureCounter from_counter (from, pos), to_counter (to, pos);

    Core::CompressedInputStream input (paramInput (config));
    
    while (readPermutation (source, input)) {
      readPermutation (pos, input);
      target = source;
      readAlignment (target, input);

      from_counter.count (source, 1.0);
      to_counter.count (target, 1.0);
    }

    std::cout << "Feature\tSource\tTarget" << std::endl;

    int i = 0;
    for (ParameterVector::const_parameter_iterator f = from.begin_p (), t = to.begin_p ();
	 f != from.end_p () && t != to.end_p ();
	 ++ f, ++ t) {
      std::ostringstream fStream;
      Core::XmlWriter fXml (fStream);
      f -> first.writeXml (from.getPOS (), fXml);
      std::string feature = fStream.str ();
      feature = feature.substr (0, feature.find ('\n'));
      std::cout << ++i << '\t'
		<<'"' << feature << '"'
		<< '\t' << f -> second
		<< '\t' << t -> second
		<< std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString FeatureCount::paramLOP ("lop", "linear ordering parameters");
Core::ParameterString FeatureCount::paramAlphabet ("alphabet", "the alphabet file");
Core::ParameterString FeatureCount::paramInput ("input", "the input file");
