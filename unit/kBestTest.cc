#include <BeforeScorer.hh>
#include <ChartFactory.hh>
#include <EqualPath.hh>
#include <PathVisitor.hh>
#include <kBestChart.hh>

#include "kBestTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( kBestTest );

void kBestTest::setUp () {
  // S_0 = 1
  schroeder_.push_back (1);
  for (int n = 1; n < 10; ++ n) {
    // S_n = S_{n-1} + \sum_{k = 0}^{n - 1} S_k S_{n - 1 - k}
    schroeder_.push_back (schroeder_.back ());
    for (int k = 0; k < n; ++ k) {
      schroeder_.back () += schroeder_ [k] * schroeder_ [n - 1 - k];
    }
  }
}

void kBestTest::tearDown () {

}

// Verifies that the numbers given here:
//  http://mathworld.wolfram.com/SchroederNumber.html
//  http://www.research.att.com/~njas/sequences/A006318
// are matched.
void kBestTest::testSchroeder () {
  CPPUNIT_ASSERT_EQUAL( 1, schroeder_ [0] );
  CPPUNIT_ASSERT_EQUAL( 2, schroeder_ [1] );
  CPPUNIT_ASSERT_EQUAL( 6, schroeder_ [2] );
  CPPUNIT_ASSERT_EQUAL( 22, schroeder_ [3] );
  CPPUNIT_ASSERT_EQUAL( 90, schroeder_ [4] );
  CPPUNIT_ASSERT_EQUAL( 394, schroeder_ [5] );
  CPPUNIT_ASSERT_EQUAL( 1806, schroeder_ [6] );
  CPPUNIT_ASSERT_EQUAL( 8558, schroeder_ [7] );
  CPPUNIT_ASSERT_EQUAL( 41586, schroeder_ [8] );
  CPPUNIT_ASSERT_EQUAL( 206098, schroeder_ [9] );
}

//
void kBestTest::testEqualPath () {
  Permute::ConstPathRef p1 = Permute::Path::arc (3, 1, 2, 1.0);
  Permute::ConstPathRef p2 = Permute::Path::arc (3, 1, 2, 1.0);
  CPPUNIT_ASSERT( Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::arc (4, 1, 2, 1.0);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::arc (3, 0, 2, 1.0);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::arc (3, 1, 3, 1.0);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::arc (3, 1, 2, 2.0);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (p2, Permute::Path::arc (4, 2, 3, 1.0), 0.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p1 = Permute::Path::connect (Permute::Path::arc (3, 1, 2, 2.0),
			       Permute::Path::arc (4, 2, 3, 1.0),
			       0.0, false);
  CPPUNIT_ASSERT( Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (2, 1, 2, 2.0),
			       Permute::Path::arc (4, 2, 3, 1.0),
			       0.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (3, 0, 2, 2.0),
			       Permute::Path::arc (4, 2, 3, 1.0),
			       0.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (3, 1, 2, 2.0),
			       Permute::Path::arc (4, 2, 4, 1.0),
			       0.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (3, 1, 2, 2.0),
			       Permute::Path::arc (4, 2, 3, 2.0),
			       0.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (3, 1, 2, 2.0),
			       Permute::Path::arc (4, 2, 3, 0.0),
			       1.0, false);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p1 = Permute::Path::connect (Permute::Path::arc (7, 0, 1, 3.0), p1, 0.0, true);
  CPPUNIT_ASSERT( ! Permute::equalPaths (p1, p2) );

  p2 = Permute::Path::connect (Permute::Path::arc (7, 0, 1, 3.0),
			       Permute::Path::connect (Permute::Path::arc (3, 1, 2, 2.0),
						       Permute::Path::arc (4, 2, 3, 1.0),
						       0.0, false),
			       0.0, true);
  CPPUNIT_ASSERT( Permute::equalPaths (p1, p2) );
}

// Asserts that each composite node in the tree contained in a visited path (1)
// returns true from normal and (2) has a right child of a different type from
// itself.
class AssertNormalVisitor : public Permute::PathVisitor {
public:
  virtual void visit (const Permute::PathComposite * pc) {
    CPPUNIT_ASSERT( pc -> normal () );
    CPPUNIT_ASSERT( pc -> type () != pc -> getRight () -> type () );
    pc -> getLeft () -> accept (this);
    pc -> getRight () -> accept (this);
  }
  virtual void visit (const Permute::Arc * arc) {
    return;
  }
};

// Verifies that the number of permutations in the k-best list for a permutation
// of size N is at most S_{N - 1}, the (N - 1)th large Schroeder number.  (After
// N = 6 this becomes very expensive to compute.)
void kBestTest::testKBest () {
  AssertNormalVisitor visitor;
  for (int N = 1; N <= 6; ++ N) {
    std::stringstream str;
    for (int i = 0; i < N; ++ i) {
      str << i << " ";
    }
    Permute::readPermutationWithAlphabet (pi_, str);
  
    Permute::ChartFactoryRef factory = Permute::ChartFactory::kbest ();
    Permute::ParseControllerRef controller = Permute::CubicParseController::create ();

    Permute::BeforeCost * bc = new Permute::BeforeCost (N);
    Permute::BeforeCostRef bcr (bc);
    for (int j = 0; j < (N - 1); ++ j) {
      bc -> setCost (0, j + 1, 1.0);
      bc -> setCost (j, N - 1, 1.0);
    }
//     bc -> setCost (2, 3, 3.0);
//     bc -> setCost (1, 2, 1.0);
//     bc -> setCost (3, 1, 1.0);
    Permute::ScorerRef scorer (new Permute::BeforeScorer (bcr, pi_));

    Permute::kBestChart kbc (factory -> chart (pi_), controller, scorer);
    kbc.permute ();

    std::vector <Permute::ConstPathRef> paths = kbc.best (schroeder_ [N - 1] + 10);
    for (std::vector <Permute::ConstPathRef>::const_iterator p = paths.begin ();
	 p != paths.end ();
	 ++ p) {
      (* p) -> accept (& visitor);
    }
    for (std::vector <Permute::ConstPathRef>::const_iterator i = paths.begin ();
	 i != paths.end ();
	 ++ i) {
      for (std::vector <Permute::ConstPathRef>::const_iterator j = i + 1;
	   j != paths.end ();
	   ++ j) {
	if (Permute::equalPaths (* i, * j)) {
	  std::cerr << "Equal paths:" << std::endl;
	  std::cerr << (* i) << std::endl;
	  std::cerr << (* j) << std::endl;
	}

	std::stringstream s1, s2;
	pi_.reorder (* i);
	s1 << pi_;
	pi_.reorder (* j);
	s2 << pi_;
	if (s1.str () == s2.str ()) {
	  std::cerr << "Equal sub-permutations:" << std::endl;
	  std::cerr << (* i) << std::endl;
	  std::cerr << (* j) << std::endl;
	}
	CPPUNIT_ASSERT_MESSAGE( s1.str (), s1.str () != s2.str () );
      }
    }
//     if (paths.size () != schroeder_ [N - 1]) {
//       std::cerr << std::endl;
//       for (std::vector <Permute::ConstPathRef>::const_iterator p = paths.begin ();
//  	   p != paths.end ();
//  	   ++ p) {
//  	pi_.reorder (* p);
//  	std::cerr << pi_ << " (" << (* p) -> getScore () << ")" << std::endl
// 		  << (* p) << std::endl;
//       }
//     }
    CPPUNIT_ASSERT_EQUAL( size_t (schroeder_ [N - 1]), paths.size () );
  }
}
