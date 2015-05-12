#include "AdjacentLossTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( AdjacentLossTest );

using namespace Permute;

void AdjacentLossTest::testIdentity () {
  Permutation target;
  integerPermutation (target, 40);

  Permutation pi (target);

  AdjacentLoss loss (target);

  CPPUNIT_ASSERT_EQUAL( 0.0, loss.score (pi) );
}

void AdjacentLossTest::testSame () {
  Permutation target;
  integerPermutation (target, 25);
  // Makes sure it's not the identity permutation.
  do {
    target.randomize ();
  } while (target [0] == 0);

  Permutation pi (target);

  AdjacentLoss loss (target);

  CPPUNIT_ASSERT_EQUAL( 0.0, loss.score (pi) );

  pi.identity ();

  CPPUNIT_ASSERT( loss.score (pi) > 0.0 );
}

void AdjacentLossTest::testDifferent () {
  Permutation target;
  integerPermutation (target, 32);
  Permutation pi (target);
  AdjacentLoss loss (target);
  
  // 17 18 ... 32 1 2 ... 16
  std::rotate (pi.begin (), pi.begin () + 16, pi.end ());
  CPPUNIT_ASSERT_EQUAL( 1.0, loss.score (pi) );

  // 25 26 ... 32 17 18 ... 24 1 2 ... 16 
  std::rotate (pi.begin (), pi.begin () + 8, pi.begin () + 16);
  CPPUNIT_ASSERT_EQUAL( 2.0, loss.score (pi) );

  // 29 ... 32 25 ... 28 17 ... 24 1 ... 16
  std::rotate (pi.begin (), pi.begin () + 4, pi.begin () + 8);
  CPPUNIT_ASSERT_EQUAL( 3.0, loss.score (pi) );
}
