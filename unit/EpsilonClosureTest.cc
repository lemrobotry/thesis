#include "EpsilonClosureTest.hh"
#include "FullCell.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( EpsilonClosureTest );

void EpsilonClosureTest::setUp () {

}
void EpsilonClosureTest::tearDown () {

}

// epsilonClosure takes a cell containing epsilon arcs and concatenates
// them until they converge.
void EpsilonClosureTest::testEpsilonClosure () {
  int N = 100;
  // Insert an epsilon arc from every state i to i + 1.
  Permute::Cell * full (new Permute::FullCell);
  for (Fsa::StateId i = 0; i < N - 1; ++ i) {
    CPPUNIT_ASSERT( full -> add (Permute::Path::epsilon (i, i + 1, -1.0f)) );
  }
  Permute::ConstCellRef closure = Permute::epsilonClosure (Permute::ConstCellRef (full));
  // After closure, there should be a path from each state to every
  // higher-numbered state, and its weight should be the distance
  // between them.
  for (Fsa::StateId i = 0; i < N - 1; ++ i) {
    // TEST j = i + 1 SEPARATELY
    
    for (Fsa::StateId j = i + 2; j < N; ++ j) {
      Permute::Cell::PathIterator iter = closure -> find (i, j, Permute::Path::KEEP);
      CPPUNIT_ASSERT( iter != closure -> end () );
      CPPUNIT_ASSERT_EQUAL( i, (* iter) -> getStart () );
      CPPUNIT_ASSERT_EQUAL( j, (* iter) -> getEnd () );
      CPPUNIT_ASSERT_EQUAL( -1.0 * (j - i), (* iter) -> getScore () );
    }
  }
  // Test the same property another way.
  for (Fsa::StateId i = 0; i < N - 1; ++ i) {
    for (Permute::Cell::PathIterator iter = closure -> begin (i, Permute::Path::KEEP);
	 iter != closure -> end ();
	 iter = closure -> next (iter, i, Permute::Path::KEEP)) {
      CPPUNIT_ASSERT_EQUAL( i, (* iter) -> getStart () );
      CPPUNIT_ASSERT( i < (* iter) -> getEnd () );
      CPPUNIT_ASSERT_EQUAL( -1.0 * ((* iter) -> getEnd () - i), (* iter) -> getScore () );
    }
  }
}
