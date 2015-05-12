#include <Core/StringUtilities.hh>

#include "Application.hh"
#include "InputData.hh"
#include "Iterator.hh"

APPLICATION

using namespace Permute;

class LowerCase : public Application {
public:
  LowerCase () :
    Application ("lower-case")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    InputData data (DEPENDENCY);

    std::istream & input = this -> input ();
    std::ostream & output = this -> output ();

    while (input >> data) {
      output << Core::convertToLowerCase (data.source ().toString ()) << std::endl
	     << data.pos () << std::endl;
      if (DEPENDENCY) {
	output << DependencyParents (data.parents ()) << std::endl
	       << data.labels () << std::endl;
      }
      output << delimit (data.target ().begin (), data.target ().end (), " ") << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
