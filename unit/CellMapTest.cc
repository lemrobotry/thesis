#include "CellMapTest.hh"
#include <FsaCellMap.hh>

#include <Fsa/Static.hh>

CPPUNIT_TEST_SUITE_REGISTRATION( CellMapTest );

void CellMapTest::setUp () {

}
void CellMapTest::tearDown () {

}

void CellMapTest::testFsaCellMap () {
  size_t N = 100;
  // Build a simple bigram automaton.
  Fsa::StaticAutomaton * fsa (new Fsa::StaticAutomaton);
  fsa -> setSemiring (Fsa::LogSemiring);
  for (size_t i = 0; i < N; ++ i) {
    Fsa::State * s (fsa -> newState ());
    for (size_t j = 0; j < N; ++ j) {
      s -> newArc (j, Fsa::LogSemiring -> one (), j);
    }
  }
  fsa -> setInitialStateId (0);
  // Make sure it maps properly.
  Fsa::ConstAutomatonRef fsa_ref (fsa);
  Permute::CellMapRef cm = Permute::FsaCellMap::mapFsa (fsa_ref).first;
  for (size_t i = 0; i < N; ++ i) {
    Permute::ConstCellRef cell = cm -> operator () (i, i);
    for (Permute::Cell::PathIterator iter = cell -> begin (); iter != cell -> end (); ++ iter) {
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (i), (* iter) -> getEnd () );
    }
    for (size_t j = 0; j < N; ++ j) {
      Permute::Cell::PathIterator iter = cell -> find (j, i, Permute::Path::NEITHER);
      CPPUNIT_ASSERT( iter != cell -> end () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (j), (* iter) -> getStart () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (i), (* iter) -> getEnd () );
      iter = cell -> begin (j, Permute::Path::NEITHER);
      CPPUNIT_ASSERT( iter != cell -> end () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (j), (* iter) -> getStart () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (i), (* iter) -> getEnd () );
      iter = cell -> next (iter, j, Permute::Path::NEITHER);
      CPPUNIT_ASSERT( iter == cell -> end () );
    }
  }
}
