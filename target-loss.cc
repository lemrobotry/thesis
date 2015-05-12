#include "Application.hh"
#include "InputData.hh"
#include "Loss.hh"

APPLICATION

using namespace Permute;

// Measures the loss of the target with respect to another target.
class TargetLoss : public Application {
public:
  TargetLoss () :
    Application ("target-loss")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    InputData test (false);
    InputData dev (false);

    Loss targetLoss;

    std::istream & input = this -> input ();
    std::istream & devInput = this -> devInput ();

    for (int sentence = 0;
	 sentence < SENTENCES
	   && input >> test
	   && devInput >> dev;
	 ++ sentence) {
      
      targetLoss.add (dev.target (), test.target ());
    }

    std::cout << "Loss:" << std::endl
	      << targetLoss << std::endl;

    return EXIT_SUCCESS;
  }
} app;
