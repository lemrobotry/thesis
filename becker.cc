// Roy W. Tromble
// 2 November 2007

#include "Application.hh"
#include "LinearOrdering.hh"

APPLICATION

using namespace Permute;

// Computes Becker's greedy ordering given a LOLIB matrix.
class Becker : public Application {
public:
  Becker () :
    Application ("becker")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::cout << "Name Score Iterations Time" << std::endl;
    int i = 0;
    for (std::vector <std::string>::const_iterator it = args.begin (); it != args.end (); ++ it) {
      this -> LOLIB_FILE = * it;
      BeforeCostRef bc (this -> lolib ());

      std::stringstream str;
      for (int i = 0; i < bc -> size (); ++ i) {
	str << i << ' ';
      }
      Permutation pi;
      readPermutationWithAlphabet (pi, str);

      Core::Timer timer;
      timer.start ();
      double score = becker_greedy (pi, bc);
      timer.stop ();
      std::cout << i++ << ' '
		<< bc -> name () << ' '
		<< score << ' '
		<< "1 "
		<< timer.user () << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
