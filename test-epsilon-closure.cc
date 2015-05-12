#include <Fsa/Application.hh>
#include <Fsa/Input.hh>
#include <Fsa/Static.hh>

#include "EpsilonClosure.hh"
#include "FsaCellMap.hh"

APPLICATION

// Computes the epsilon closure of an automaton from an input file and prints
// the start and end states of the paths in the closure, along with path
// weights.
class TestEpsilonClosure : public Fsa::Application {
private:
  static Core::ParameterString paramFsaFile;
public:
  TestEpsilonClosure () {
    setTitle ("test-epsilon-closure");
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }
  Fsa::ConstAutomatonRef read (const std::string & file) {
    Fsa::StaticAutomaton * fsa = new Fsa::StaticAutomaton;
    Fsa::read (fsa, file);
    return Fsa::ConstAutomatonRef (fsa);
  }
  int main (const std::vector <std::string> & args) {
    Fsa::ConstAutomatonRef fsa = read (paramFsaFile (config));
    Permute::FsaCellMap cellMap (fsa);
    Permute::ConstCellRef cell = Permute::epsilonClosure (cellMap (Fsa::Epsilon));
    for (Permute::Cell::PathIterator path = cell -> begin (); path != cell -> end (); ++ path) {
      std::cout << "[" << (* path) -> getStart ()
		<< ", " << (* path) -> getEnd ()
		<< "]: " << (* path) -> getScore ()
		<< std::endl;
    }
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString TestEpsilonClosure::paramFsaFile ("fsa", "the automaton file", "-");
