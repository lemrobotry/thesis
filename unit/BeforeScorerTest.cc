#include <Core/TextStream.hh>
#include <ParseController.hh>
#include "BeforeScorerTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( BeforeScorerTest );

void BeforeScorerTest::setUp () {
  Permute::integerPermutation (pi, 10);

  Permute::BeforeCostRef bc (new Permute::BeforeCost (pi.size (), "BeforeScorerTest::setUp"));

  scorer = new Permute::BeforeScorer (bc, pi);
}

void BeforeScorerTest::tearDown () {
  delete scorer;
}

void BeforeScorerTest::testBinomial () {
  int sum = 0;
  for (int i = 1; i < 1000; ++ i) {
    CPPUNIT_ASSERT_EQUAL( sum, scorer -> binomial (i) );
    sum += i;
  }
}

void BeforeScorerTest::testIndex () {
  int index = 0;
  for (int i = 0; i <= 10; ++ i) {
    for (int j = i + 1; j <= 10; ++ j) {
      for (int k = j + 1; k <= 10; ++ k) {
	std::ostringstream out;
	out << "index (" << i << ", " << j << ", " << k << ")";
	CPPUNIT_ASSERT_EQUAL_MESSAGE( out.str (), index, scorer -> index (i, j, k) );
	++ index;
      }
    }
  }
}

// Reads a cost matrix from LOLIB.  Verifies that computing the scores via the
// default (i+1,j,k) + (i,j,k-1) - (i+1,j,k-1) gives the same result as
// computing them via the alternative method (i,j,k-1) + (i,j-1,k) -
// (i,j-1,k-1).
void BeforeScorerTest::testLeftAnchorRecurrence () {
  Core::TextInputStream input ("be75eec.mat");
  Permute::BeforeCostRef bc (Permute::readLOLIB (input));
  CPPUNIT_ASSERT_EQUAL( size_t (50), bc -> size () );

  std::stringstream str;
  for (int i = 0; i < bc -> size (); ++ i) {
    str << i << ' ';
  }
  Permute::Permutation p;
  Permute::readPermutationWithAlphabet (p, str);
  
  delete scorer;
  scorer = new Permute::BeforeScorer (bc, p);
  scorer -> compute (Permute::CubicParseController::create ());

  for (int i = 0; i < bc -> size (); ++ i) {
    for (int j = i + 1; j < bc -> size (); ++ j) {
      for (int k = j + 1; k < bc -> size (); ++ k) {
	double standard =
	  scorer -> score (i, j, k - 1)
	  + scorer -> score (i + 1, j, k)
	  - scorer -> score (i + 1, j, k - 1)
	  + bc -> cost (p [i], p [k - 1]);
	double alternate =
	  scorer -> score (i, j, k - 1)
	  + scorer -> score (i, j - 1, k)
	  - scorer -> score (i, j - 1, k - 1)
	  + bc -> cost (p [j - 1], p [k - 1]);
	std::ostringstream out;
	out << "(" << i << ", " << j << ", " << k << ")";
	CPPUNIT_ASSERT_EQUAL_MESSAGE( out.str(), standard, alternate );
	standard =
	  scorer -> score (k - 1, j, i)
	  + scorer -> score (k, j, i + 1)
	  - scorer -> score (k - 1, j, i + 1)
	  + bc -> cost (p [k - 1], p [i]);
	alternate =
	  scorer -> score (k - 1, j, i)
	  + scorer -> score (k, j - 1, i)
	  - scorer -> score (k - 1, j - 1, i)
	  + bc -> cost (p [k - 1], p [j - 1]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE( out.str (), standard, alternate );
      }
    }
  }
}
