#include "CellTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( CellTest );

void CellTest::setUp () {
  best = Permute::CellRef (new Permute::BestCell ());
  full = Permute::CellRef (new Permute::FullCell ());
  arc = Permute::Path::epsilon (13, 17, 0.0f);
}
void CellTest::tearDown () {
}
void CellTest::addArc () {
  CPPUNIT_ASSERT( best -> add (arc) );
  CPPUNIT_ASSERT( full -> add (arc) );
}

void CellTest::testEmpty () {
  CPPUNIT_ASSERT( best -> empty () );
  CPPUNIT_ASSERT( full -> empty () );
  addArc ();
  CPPUNIT_ASSERT( ! best -> empty () );
  CPPUNIT_ASSERT( ! full -> empty () );
}
void CellTest::testClear () {
  addArc ();
  CPPUNIT_ASSERT( ! best -> empty () );
  CPPUNIT_ASSERT( ! full -> empty () );
  best -> clear ();
  full -> clear ();
  CPPUNIT_ASSERT( best -> empty () );
  CPPUNIT_ASSERT( full -> empty () );
}
void CellTest::testBegin () {
  CPPUNIT_ASSERT( best -> begin () == best -> end () );
  CPPUNIT_ASSERT( full -> begin () == full -> end () );
  addArc ();
  CPPUNIT_ASSERT( * best -> begin () == arc );
  CPPUNIT_ASSERT( * full -> begin () == arc );
  CPPUNIT_ASSERT( ++ best -> begin () == best -> end () );
  CPPUNIT_ASSERT( ++ full -> begin () == full -> end () );
}
void CellTest::testBegin1 () {
  CPPUNIT_ASSERT( best -> begin (13, Permute::Path::NEITHER) == best -> end () );
  CPPUNIT_ASSERT( full -> begin (13, Permute::Path::NEITHER) == full -> end () );
  addArc ();
  CPPUNIT_ASSERT( * best -> begin (13, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( * full -> begin (13, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( best -> next (best -> begin (13, Permute::Path::NEITHER), 13, Permute::Path::NEITHER) == best -> end () );
  CPPUNIT_ASSERT( full -> next (full -> begin (13, Permute::Path::NEITHER), 13, Permute::Path::NEITHER) == full -> end () );
}
void CellTest::testFind () {
  CPPUNIT_ASSERT( best -> find (13, 17, Permute::Path::NEITHER) == best -> end () );
  CPPUNIT_ASSERT( full -> find (13, 17, Permute::Path::NEITHER) == full -> end () );
  addArc ();
  CPPUNIT_ASSERT( * best -> find (13, 17, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( * full -> find (13, 17, Permute::Path::NEITHER) == arc );
}
/*
void CellTest::testRBegin () {
  CPPUNIT_ASSERT( best -> rbegin () == best -> rend () );
  CPPUNIT_ASSERT( full -> rbegin () == full -> rend () );
  addArc ();
  CPPUNIT_ASSERT( * best -> rbegin () == Permute::Path::nullPath () );
  CPPUNIT_ASSERT( * full -> rbegin () == Permute::Path::nullPath () );
  CPPUNIT_ASSERT( ++ best -> rbegin () == best -> rend () );
  CPPUNIT_ASSERT( ++ full -> rbegin () == full -> rend () );
}
void CellTest::testRBegin1 () {
  CPPUNIT_ASSERT( best -> rbegin (17) == best -> rend () );
  CPPUNIT_ASSERT( full -> rbegin (17) == full -> rend () );
  addArc ();
  CPPUNIT_ASSERT( * best -> rbegin (17) == arc );
  CPPUNIT_ASSERT( * full -> rbegin (17) == arc );
  CPPUNIT_ASSERT( ++ best -> rbegin (17) == best -> rend (17) );
  CPPUNIT_ASSERT( ++ full -> rbegin (13) == full -> rend (17) );
}
*/
void CellTest::testAdd () {
  // Best keeps only one path, so adding a better one should make the
  // other disappear.
  addArc ();
  CPPUNIT_ASSERT( ! best -> add (Permute::Path::epsilon (0, 1, -0.5f)) );
  CPPUNIT_ASSERT( * best -> find (13, 17, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( best -> add (Permute::Path::epsilon (1, 3, 0.5f)) );
  CPPUNIT_ASSERT( best -> find (13, 17, Permute::Path::NEITHER) == best -> end () );
  // Full keeps only one path with each start/end point, so adding a
  // better one should make the other disappear.
  CPPUNIT_ASSERT( * full -> find (13, 17, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( full -> add (Permute::Path::epsilon (1, 3, 0.5f)) );
  CPPUNIT_ASSERT( ! full -> add (Permute::Path::epsilon (1, 3, 0.2f)) );
  CPPUNIT_ASSERT( * full -> find (13, 17, Permute::Path::NEITHER) == arc );
  CPPUNIT_ASSERT( full -> add (Permute::Path::epsilon (13, 17, 0.3f)) );
  CPPUNIT_ASSERT( * full -> find (13, 17, Permute::Path::NEITHER) != arc );
}
void CellTest::testBuildFunction (CellTest::BuildFunction build) {
  CPPUNIT_ASSERT( full -> add (arc) );
  build (best, Permute::ConstCellRef (full), Permute::ConstCellRef (full), 0.0f, false, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( best -> empty () );
  Permute::CellRef full2 (new Permute::FullCell ());
  CPPUNIT_ASSERT( full2 -> add (Permute::Path::epsilon (17, 11, 0.0f)) );
  build (best, Permute::ConstCellRef (full), Permute::ConstCellRef (full2), 0.0f, false, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( ! best -> empty () );
  CPPUNIT_ASSERT( best -> find (13, 11, Permute::Path::KEEP) != best -> end () );

  // Insert 1000 paths into full and full2.
  best -> clear ();
  full -> clear ();
  full2 -> clear ();
  CPPUNIT_ASSERT_EQUAL( size_t (0), best -> size () );
  CPPUNIT_ASSERT_EQUAL( size_t (0), full -> size () );
  for (int i = 0; i < 1000; ++ i) {
    CPPUNIT_ASSERT( full -> add (Permute::Path::epsilon (1, i, float (i))) );
    CPPUNIT_ASSERT( full2 -> add (Permute::Path::epsilon (i, 2, float (i))) );
  }
  // Build best from full and full2.
  build (best, Permute::ConstCellRef (full), Permute::ConstCellRef (full2), 0.0f, false, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( ! best -> empty () );
  CPPUNIT_ASSERT_EQUAL( size_t (1), best -> size () );
  CPPUNIT_ASSERT_EQUAL( size_t (1000), full -> size () );
  // Check the properties of the path in best.
  Permute::Cell::PathIterator best_iter = best -> find (1, 2, Permute::Path::KEEP);
  CPPUNIT_ASSERT( best_iter != best -> end () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (1), (* best_iter) -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (2), (* best_iter) -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( 1998.0, (* best_iter) -> getScore () );
  // Add 2000 more paths to full2.
  CPPUNIT_ASSERT_EQUAL( size_t (1000), full2 -> size () );
  for (int i = 0; i < 2000; ++ i) {
    CPPUNIT_ASSERT( full2 -> add (Permute::Path::epsilon (i, 1, float (i))) );
  }
  CPPUNIT_ASSERT_EQUAL( size_t (3000), full2 -> size () );
  // Build best again, but with opposite order.
  build (best, Permute::ConstCellRef (full2), Permute::ConstCellRef (full), 0.0f, true, Permute::Path::NEITHER);
  CPPUNIT_ASSERT_EQUAL( size_t (1), best -> size () );
  CPPUNIT_ASSERT( ! best -> empty () );
  // Check the properties of the new best path.
  best_iter = best -> find (1999, 999, Permute::Path::SWAP);
  CPPUNIT_ASSERT( best_iter != best -> end () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (1999), (* best_iter) -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (999), (* best_iter) -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( 2998.0, (* best_iter) -> getScore () );
}
void CellTest::testBuild () {
  testBuildFunction (Permute::Cell::build);
}
/*
void CellTest::testNewBuild () {
  testBuildFunction (Permute::Cell::NEWbuild);
}
*/
void CellTest::testGetBestPath () {
  testAdd ();
  Permute::ConstPathRef bestBest = Permute::Cell::getBestPath (Permute::ConstCellRef (best));
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (1), bestBest -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (3), bestBest -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( 0.5, bestBest -> getScore () );
  Permute::ConstPathRef fullBest = Permute::Cell::getBestPath (Permute::ConstCellRef (full));
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (1), fullBest -> getStart () );
  CPPUNIT_ASSERT_EQUAL( Fsa::StateId (3), fullBest -> getEnd () );
  CPPUNIT_ASSERT_EQUAL( 0.5, fullBest -> getScore () );
}

void CellTest::stressTest () {
  CPPUNIT_ASSERT_EQUAL( size_t (0), best -> size () );
  CPPUNIT_ASSERT_EQUAL( size_t (0), full -> size () );
  // Add one million paths.
  for (int j = 0; j < 1000; ++ j) {
    for (int i = 0; i < 1000; ++ i) {
      CPPUNIT_ASSERT( full -> add (Permute::Path::epsilon (i, j, float (j - i))) );
      best -> add (Permute::Path::epsilon (i, j, float (j - i)));
    }
  }
  CPPUNIT_ASSERT_EQUAL( size_t (1), best -> size () );
  CPPUNIT_ASSERT_EQUAL( size_t (1000000), full -> size () );
  // Test that full has all the paths.
  for (int i = 0; i < 1000; ++ i) {
    for (int j = 0; j < 1000; ++ j) {
      Permute::Cell::PathIterator iter = full -> find (i, j, Permute::Path::NEITHER);
      CPPUNIT_ASSERT( iter != full -> end () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (i), (* iter) -> getStart () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (j), (* iter) -> getEnd () );
      CPPUNIT_ASSERT_EQUAL( double (j - i), (* iter) -> getScore () );
    }
  }
  // Test that there are 1000 elements for a given start state.
  Permute::Cell::PathIterator iter = full -> begin (159, Permute::Path::NEITHER);
  for (int i = 0; i < 1000; ++ i) {
    CPPUNIT_ASSERT( iter != full -> end () );
    CPPUNIT_ASSERT_EQUAL( Fsa::StateId (159), (* iter) -> getStart () );
    iter = full -> next (iter, 159, Permute::Path::NEITHER);
  }
  CPPUNIT_ASSERT( iter == full -> end () );
}
