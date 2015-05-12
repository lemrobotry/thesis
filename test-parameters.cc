#include <Fsa/Application.hh>
#include "Parameter.hh"

APPLICATION

using namespace Permute;

// Reads in a ParameterVector from an XML file and writes out the weight of the
// (NN, NN, 1) feature.
class TestParameters : public Fsa::Application {
private:
  static Core::ParameterString paramFile;
public:
  TestParameters () {
    setTitle ("test-parameters");
    setDefaultLoadConfigurationFile (false);
  }
  
  int main (const std::vector <std::string> & args) {
    ParameterVector pv;
    ParameterVectorXmlParser parser (config, pv);
    if (parser.parseFile (paramFile (config))) {
      POS nn = pv.getPOS () -> index ("NN");
      double w = sum (pv, nn, nn, 1);
      std::cout << w << std::endl;
      writeXml (pv, std::cout);
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString TestParameters::paramFile ("params", "the parameter file", "");
