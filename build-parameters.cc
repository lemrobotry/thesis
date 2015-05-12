#include <Core/CompressedStream.hh>
#include <Fsa/AlphabetXml.hh>
#include <Fsa/Application.hh>
#include "Parameter.hh"

APPLICATION

using namespace Permute;

// Reads a part-of-speech alphabet and constructs all simple (POS, POS,
// distance) features.  If the --backoff option is set, includes *ANY* as a POS
// matching any symbol.  The --threshold option determines the maximum distance
// for which a separate feature is included.  Writes the resulting parameter XML
// file to the --output option.
class BuildParameters : public Fsa::Application {
private:
  static Core::ParameterString paramPosAlphabet;
  static Core::ParameterInt paramThreshold;
  static Core::ParameterBool paramBackoff;
  static Core::ParameterString paramOutput;
public:
  BuildParameters () {
    setTitle ("build-parameters");
    setDefaultLoadConfigurationFile (false);
  }

  void distance (ParameterVector & pv, Feature & phi) {
    for (int d = 1; d <= paramThreshold (config); ++ d) {
      phi.setDistance (d);
      phi.setComparison (CompEquals);
      pv.push_back (phi, 0.0);
    }
    phi.setComparison (CompGreaterThan);
    pv.push_back (phi, 0.0);
  }

  void secondPOS (Fsa::ConstAlphabetRef pos, ParameterVector & pv, Feature & phi) {
    for (Fsa::Alphabet::const_iterator p2 = pos -> begin (); p2 != pos -> end (); ++ p2) {
      phi.setSecondPOS (p2);
      distance (pv, phi);
    }
    if (paramBackoff (config)) {
      phi.setSecondPOS (Fsa::Any);
      distance (pv, phi);
    }
  }

  int main (const std::vector <std::string> & args) {
    Fsa::ConstAlphabetRef pos = Fsa::readAlphabet (paramPosAlphabet (config));
    ParameterVector pv;
    pv.setPOS (pos);
    
    Feature phi;
    for (Fsa::Alphabet::const_iterator p1 = pos -> begin (); p1 != pos -> end (); ++ p1) {
      phi.setFirstPOS (p1);
      secondPOS (pos, pv, phi);
    }
    if (paramBackoff (config)) {
      phi.setFirstPOS (Fsa::Any);
      secondPOS (pos, pv, phi);
    }

    Core::CompressedOutputStream co (paramOutput (config));
    writeXml (pv, co);
    co.close ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString BuildParameters::paramPosAlphabet ("pos", "the POS alphabet");
Core::ParameterInt BuildParameters::paramThreshold ("threshold", "the distance threshold", 5, 0);
Core::ParameterBool BuildParameters::paramBackoff ("backoff", "include parameters with *ANY*", false);
Core::ParameterString BuildParameters::paramOutput ("output", "the parameter output file");
