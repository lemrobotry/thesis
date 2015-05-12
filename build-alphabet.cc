#include <Core/CompressedStream.hh>
#include <Core/XmlStream.hh>

#include <Fsa/AlphabetXml.hh>
#include <Fsa/Application.hh>

APPLICATION

// Constructs an Fsa::Alphabet that contains all the symbols in the given --text
// file, and stores the result in the given --alphabet file.
class BuildAlphabet : public Fsa::Application {
private:
  static Core::ParameterString paramText;
  static Core::ParameterString paramAlphabet;
public:
  BuildAlphabet () : Fsa::Application () {
    setTitle ("build-alphabet");
    setDefaultLoadConfigurationFile (false);
  }

  int main (const std::vector <std::string> & args) {

    Fsa::StaticAlphabet * alphabet = new Fsa::StaticAlphabet;

    Core::CompressedInputStream text (paramText (config));
    std::string symbol;

    while (text >> symbol) {
      if (alphabet -> addSymbol (symbol) == Fsa::InvalidLabelId) {
	std::cerr << "Invalid symbol: " << symbol << std::endl;
      }
    }

    text.close ();

    Core::CompressedOutputStream output (paramAlphabet (config));
    Core::XmlWriter xml (output);

    xml << Core::XmlOpen ("alphabet") << "\n";
    alphabet -> writeXml (xml);
    xml << Core::XmlClose ("alphabet") << "\n";

    output.close ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString BuildAlphabet::paramText ("text", "text file containing alphabet symbols");
Core::ParameterString BuildAlphabet::paramAlphabet ("alphabet", "alphabet xml file");
