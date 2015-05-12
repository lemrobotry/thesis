#include "PathTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( PathTest );

void PathTest::setUp () {
  null = Permute::Path::nullPath ();
  lower = Permute::Path::lowerBound (9);
  rlower = Permute::Path::rLowerBound (11);
  upper = Permute::Path::upperBound (10);
  rupper = Permute::Path::rUpperBound (11);
  forsearch = Permute::Path::forSearch (7, 8);
  arc1 = Permute::Path::arc (51, 9, 10, 0.5);
  arc2 = Permute::Path::arc (57, 10, 11, 0.6);
  connect = Permute::Path::connect (arc1, arc2, 0.1, false);
  swap = Permute::Path::connect (arc1, arc2, 0.2, true);
  epsilon = Permute::Path::epsilon (11, 12, 0.7);
  final = Permute::Path::final (12, 0.8);
  addscore = Permute::Path::addScore (arc2, 0.9);
}
void PathTest::tearDown () {
}

void PathTest::testGetStart () {
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (9), lower -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (10), upper -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (7), forsearch -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (9), arc1 -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (10), arc2 -> getStart () );
  CPPUNIT_ASSERT_EQUAL( arc1 -> getStart (), connect -> getStart () );
  CPPUNIT_ASSERT_EQUAL( arc1 -> getStart (), swap -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (11), epsilon -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (12), final -> getStart () );
  CPPUNIT_ASSERT_EQUAL( arc2 -> getStart (), addscore -> getStart () );
}
void PathTest::testGetEnd () {
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (11), rlower -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (11), rupper -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (8), forsearch -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (10), arc1 -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (11), arc2 -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( arc2 -> getEnd (), connect -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( arc2 -> getEnd (), swap -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (12), epsilon -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( arc2 -> getEnd (), addscore -> getEnd () );
}
void PathTest::testGetScore () {
  CPPUNIT_ASSERT_EQUAL( 0.5, arc1 -> getScore () );
  CPPUNIT_ASSERT_EQUAL( 0.6, arc2 -> getScore () );
  CPPUNIT_ASSERT_EQUAL( arc1 -> getScore () + arc2 -> getScore () + 0.1, connect -> getScore () );
  CPPUNIT_ASSERT_EQUAL( arc1 -> getScore () + arc2 -> getScore () + 0.2, swap -> getScore () );
  CPPUNIT_ASSERT_EQUAL( 0.7, epsilon -> getScore () );
  CPPUNIT_ASSERT_EQUAL( 0.8, final -> getScore () );
  CPPUNIT_ASSERT_EQUAL( arc2 -> getScore () + 0.9, addscore -> getScore () );
}
void PathTest::testNormal () {
  CPPUNIT_ASSERT( connect -> normal () );
  Permute::ConstPathRef connect2 = Permute::Path::connect (epsilon, final, 0.0, false),
    connect3 = Permute::Path::connect (connect, connect2, 0.0, false);
  CPPUNIT_ASSERT( connect2 -> normal () );
  CPPUNIT_ASSERT( ! connect3 -> normal () );

  CPPUNIT_ASSERT( swap -> normal () );
  Permute::ConstPathRef swap2 = Permute::Path::connect (final, epsilon, 0.0, true),
    swap3 = Permute::Path::connect (swap2, swap, 0.0, true);
  CPPUNIT_ASSERT( swap2 -> normal () );
  CPPUNIT_ASSERT( ! swap3 -> normal () );

  // Normal: ((1 2) [3 4]) -> 1 2 4 3
  CPPUNIT_ASSERT( Permute::Path::connect (connect, swap2, 0.0, false) -> normal () );
  // Abnormal: [[1 2] (3 4)] -> 3 4 2 1 --- Normal: [1 [2 (3 4)]]
  CPPUNIT_ASSERT( ! Permute::Path::connect (connect2, swap, 0.0, true) -> normal () );
  // Abnormal: ([1 2] (3 4)) -> 2 1 3 4 --- Normal: (([1 2] 3) 4)
  CPPUNIT_ASSERT( ! Permute::Path::connect (swap, connect2, 0.0, false) -> normal () );
  CPPUNIT_ASSERT( Permute::Path::connect (Permute::Path::connect (swap, epsilon, 0.0, false), final, 0.0, false) -> normal () );
  // Normal: [(1 2) [3 4]] -> 4 3 1 2 --- Abnormal: [[(1 2) 3] 4]
  CPPUNIT_ASSERT( Permute::Path::connect (swap2, connect, 0.0, true) -> normal () );
  CPPUNIT_ASSERT( ! Permute::Path::connect (final, Permute::Path::connect (epsilon, connect, 0.0, true), 0.0, true) -> normal () );
}
void PathTest::testLessThan () {
  Permute::Path::LessThan lt;
  CPPUNIT_ASSERT( lt (lower, arc1) );
  CPPUNIT_ASSERT( ! lt (arc1, lower) );
  CPPUNIT_ASSERT( lt (lower, connect) );
  CPPUNIT_ASSERT( ! lt (connect, lower) );
  CPPUNIT_ASSERT( lt (arc2, upper) );
  CPPUNIT_ASSERT( ! lt (upper, arc2) );
  CPPUNIT_ASSERT( lt (arc1, arc2) );
  CPPUNIT_ASSERT( ! lt (arc2, arc1) );
  CPPUNIT_ASSERT( lt (arc1, connect) );
  CPPUNIT_ASSERT( ! lt (connect, arc1) );
  CPPUNIT_ASSERT( lt (arc1, epsilon) );
  CPPUNIT_ASSERT( ! lt (epsilon, arc1) );
}
void PathTest::testRLessThan () {
  Permute::Path::rLessThan rlt;
  CPPUNIT_ASSERT( rlt (rlower, arc2) );
  CPPUNIT_ASSERT( ! rlt (arc2, rlower) );
  CPPUNIT_ASSERT( rlt (rlower, connect) );
  CPPUNIT_ASSERT( ! rlt (connect, rlower) );
  CPPUNIT_ASSERT( rlt (arc2, rupper) );
  CPPUNIT_ASSERT( ! rlt (rupper, arc2) );
  CPPUNIT_ASSERT( rlt (connect, rupper) );
  CPPUNIT_ASSERT( ! rlt (rupper, connect) );
  CPPUNIT_ASSERT( rlt (arc1, arc2) );
  CPPUNIT_ASSERT( ! rlt (arc2, arc1) );
  CPPUNIT_ASSERT( rlt (arc1, connect) );
  CPPUNIT_ASSERT( ! rlt (connect, arc1) );
  CPPUNIT_ASSERT( rlt (connect, arc2) );
  CPPUNIT_ASSERT( ! rlt (arc2, connect) );
  CPPUNIT_ASSERT( rlt (arc1, epsilon) );
  CPPUNIT_ASSERT( ! rlt (epsilon, arc1) );
}
