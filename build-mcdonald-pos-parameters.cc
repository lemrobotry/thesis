#include "Application.hh"
#include "Parameter.hh"

APPLICATION

using namespace Permute;

class McDonaldPOSParameters : public Application {
public:
  McDonaldPOSParameters () : Application ("build-mcdonald-pos-parameters") {}

  int main (const std::vector <std::string> & args) {
    Fsa::ConstAlphabetRef POS = this -> alphabet ();
    ParameterVector pv;
    pv.setPOS (POS);

    Permutation source, pos (POS), target;

    int sentence = 1;
    for (std::istream & in = this -> input (); Permute::readPermutationWithAlphabet (source, in); ++sentence) {
      if (sentence % 10000 == 0) {
	std::cerr << sentence << ": " << pv.size () << std::endl;
      }
      
      readPermutation (pos, in);
      target = source;
      readAlignment (target, in);

      Feature phi;
      Feature phi_d;
      size_t N = pos.size () - 1;
      for (size_t i = 0; i < N; ++ i) {
	phi.setFirstPOS (pos.label (i));
	for (size_t j = i + 1; j <= N; ++ j) {
	  // Basic
	  phi.setSecondPOS (pos.label (j));
	  pv [phi] ++;
	  phi_d = phi;
	  int distance = j - i;
	  Comparison op = CompEquals;
	  if (distance > 4) {
	    distance = 4;
	    op = CompGreaterThan;
	  }
	  if (distance > 9) {
	    distance = 9;
	  }
	  phi_d.setDistance (distance);
	  phi_d.setComparison (op);
	  pv [phi_d] ++;
	  // Backoff
	  phi.setFirstPOS (Fsa::Any);
	  phi_d.setPOS (phi);
	  pv [phi] ++;
	  pv [phi_d] ++;
	  phi.setFirstPOS (pos.label (i));
	  phi.setSecondPOS (Fsa::Any);
	  phi_d.setPOS (phi);
	  pv [phi] ++;
	  pv [phi_d] ++;
	  phi.setSecondPOS (pos.label (j));
	  // Between
	  for (int b = i + 1; b < j; ++ b) {
	    phi.setBetweenPOS (pos.label (b));
	    phi_d.setPOS (phi);
	    pv [phi] ++;
	    pv [phi_d] ++;
	  }
	  phi.setBetweenPOS (Fsa::Any);
	  // Extended
	  if (i == 0) {
	    phi.setFirstPOSm1 (Fsa::InvalidLabelId);
	  } else {
	    phi.setFirstPOSm1 (pos.label (i - 1));
	  }
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fm1
	  pv [phi_d] ++;
	  phi.setSecondPOSm1 (pos.label (j - 1));
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fm1 sm1
	  pv [phi_d] ++;
	  phi.setSecondPOSm1 (Fsa::Any);
	  if (j == N) {
	    phi.setSecondPOSp1 (Fsa::InvalidLabelId);
	  } else {
	    phi.setSecondPOSp1 (pos.label (j + 1));
	  }
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fm1 sp1
	  pv [phi_d] ++;
	  phi.setFirstPOSm1 (Fsa::Any);
	  phi_d.setPOS (phi);
	  pv [phi] ++; // sp1
	  pv [phi_d] ++;
	  phi.setFirstPOSp1 (pos.label (i + 1));
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fp1 sp1
	  pv [phi_d] ++;
	  phi.setSecondPOSp1 (Fsa::Any);
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fp1
	  pv [phi_d] ++;
	  phi.setSecondPOSm1 (pos.label (j - 1));
	  phi_d.setPOS (phi);
	  pv [phi] ++; // fp1 sm1
	  pv [phi_d] ++;
	  phi.setFirstPOSp1 (Fsa::Any);
	  phi_d.setPOS (phi);
	  pv [phi] ++; // sm1
	  pv [phi_d] ++;
	}
      }
    }

    this -> outputParameters (pv);
    
    return EXIT_SUCCESS;
  }
} app;
