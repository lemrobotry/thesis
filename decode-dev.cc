#include "Application.hh"
#include "ChartFactory.hh"
#include "PV.hh"

APPLICATION

// Reads a PV and calls Application::decodeDev to decode the dev set using that
// PV and output the results of the DEV_COMMAND.
class DecodeDev : public Permute::Application {
public:
  DecodeDev () :
    Permute::Application ("decode-dev") {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    std::cout << "ITERATE_SEARCH? "
	      << (ITERATE_SEARCH ? "true" : "false")
	      << std::endl;
    
    Permute::PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    this -> decodeDev (pv);

    return EXIT_SUCCESS;
  }
} app;
