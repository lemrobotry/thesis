#include "CellHashTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( CellHashTest );

void CellHashTest::setUp () {
  cell = new Permute::CellHash;
  arc_5_12 = Permute::Path::epsilon (5, 12, 2.0);
  arc_8_15 = Permute::Path::epsilon (8, 15, 3.0);
}
void CellHashTest::tearDown () {
  delete cell;
}

void CellHashTest::testEmpty () {
  CPPUNIT_ASSERT( cell -> empty () );
  cell -> insert (arc_5_12);
  CPPUNIT_ASSERT( ! cell -> empty () );
}
void CellHashTest::testClear () {
  cell -> insert (arc_5_12);
  CPPUNIT_ASSERT( ! cell -> empty () );
  cell -> clear ();
  CPPUNIT_ASSERT( cell -> empty () );
}
void CellHashTest::testInsert () {
  CPPUNIT_ASSERT( ! cell -> insert (arc_5_12) );
  CPPUNIT_ASSERT( cell -> insert (arc_5_12) );
}
void CellHashTest::testFind () {
  CPPUNIT_ASSERT( cell -> end () == cell -> find (5, 12, Permute::Path::NEITHER) );
  cell -> insert (arc_5_12);
  CPPUNIT_ASSERT( cell -> end () != cell -> find (5, 12, Permute::Path::NEITHER) );
  CPPUNIT_ASSERT( arc_5_12 == * cell -> find (5, 12, Permute::Path::NEITHER) );
}
void CellHashTest::testErase () {
  cell -> insert (arc_5_12);
  Permute::CellHash::iterator i = cell -> find (5, 12, Permute::Path::NEITHER);
  * i = Permute::Path::epsilon (5, 12, 8.0);
  CPPUNIT_ASSERT_EQUAL( 8.0, (* cell -> find (5, 12, Permute::Path::NEITHER)) -> getScore () );
}
void CellHashTest::testIterator () {
  CPPUNIT_ASSERT( cell -> begin () == cell -> end () );
  cell -> insert (arc_5_12);
  cell -> insert (arc_8_15);
  Permute::CellHash::const_iterator i = cell -> begin ();
  CPPUNIT_ASSERT( i ++ != cell -> end () );
  CPPUNIT_ASSERT( i ++ != cell -> end () );
  CPPUNIT_ASSERT( i == cell -> end () );
}
void CellHashTest::testBoundIterator () {
  CPPUNIT_ASSERT( cell -> lower_bound (5, Permute::Path::NEITHER) == cell -> end () );
  cell -> insert (arc_5_12);
  cell -> insert (arc_8_15);
  Permute::ConstPathRef arc_5_13 = Permute::Path::epsilon (5, 13, 4.0);
  cell -> insert (arc_5_13);
  Permute::CellHash::const_iterator i = cell -> lower_bound (5, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( i != cell -> end () );
  CPPUNIT_ASSERT( * i == arc_5_13 );
  i = cell -> next (i, 5, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( i != cell -> end () );
  CPPUNIT_ASSERT( * i == arc_5_12 );
  i = cell -> next (i, 5, Permute::Path::NEITHER);
  CPPUNIT_ASSERT( i == cell -> end () );
}
void CellHashTest::testRIterator () {
  CPPUNIT_FAIL( "Not implemented!" );
}
void CellHashTest::testRBoundIterator () {
  CPPUNIT_FAIL( "Not implemented!" );
}
void CellHashTest::stressTest () {
  int N = 100;
  int TEST = 31;
  CPPUNIT_ASSERT_EQUAL( size_t (0), cell -> size () );
  for (int i = 0; i < N; ++ i) {
    for (int j = 0; j < N; ++ j) {
      CPPUNIT_ASSERT( ! cell -> insert (Permute::Path::epsilon (j, i, float (j - i))) );
    }
  }
  CPPUNIT_ASSERT_EQUAL( size_t (N * N), cell -> size () );
  // Test that duplicate insertion fails.
  for (int i = 0; i < N; ++ i) {
    for (int j = 0; j < N; ++ j) {
      CPPUNIT_ASSERT( cell -> insert (Permute::Path::epsilon (i, j, float (j - i))) );
    }
  }
  CPPUNIT_ASSERT_EQUAL( size_t (N * N), cell -> size () );
  // Test that we can find everything we put in.
  for (int i = 0; i < N; ++ i) {
    for (int j = 0; j < N; ++ j) {
      Permute::CellHash::const_iterator iter = const_cast <const Permute::CellHash *> (cell) -> find (i, j, Permute::Path::NEITHER);
      CPPUNIT_ASSERT( iter != cell -> end () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (i), (* iter) -> getStart () );
      CPPUNIT_ASSERT_EQUAL( Fsa::StateId (j), (* iter) -> getEnd () );
      CPPUNIT_ASSERT_EQUAL( double (i - j), (* iter) -> getScore () );
    }
  }
  // Test that there are N elements for a given start state.
  Permute::CellHash::const_iterator iter = cell -> lower_bound (TEST, Permute::Path::NEITHER);
  for (int i = 0; i < N; ++ i) {
    CPPUNIT_ASSERT( iter != cell -> end () );
    CPPUNIT_ASSERT_EQUAL( Fsa::StateId (TEST), (* iter) -> getStart () );
    iter = cell -> next (iter, TEST, Permute::Path::NEITHER);
  }
  CPPUNIT_ASSERT( iter == cell -> end () );
}

void CellHashTest::stressTest2 () {
  int N = 20;
  CPPUNIT_ASSERT_EQUAL( size_t (0), cell -> size () );
  for (Fsa::StateId i = 0; i < N; ++ i) {
    CPPUNIT_ASSERT( ! cell -> insert (Permute::Path::epsilon (i, i + 1, -1.0f)) );
  }
  CPPUNIT_ASSERT_EQUAL( size_t (N), cell -> size () );
  for (Fsa::StateId i = 0; i < N; ++ i) {
    Permute::CellHash::const_iterator iter = cell -> lower_bound (i, Permute::Path::NEITHER);
    CPPUNIT_ASSERT_EQUAL( i, (* iter) -> getStart () );
    CPPUNIT_ASSERT_EQUAL( i + 1, (* iter) -> getEnd () );
    CPPUNIT_ASSERT_EQUAL( -1.0, (* iter) -> getScore () );
    iter = cell -> next (iter, i, Permute::Path::NEITHER);
    CPPUNIT_ASSERT( iter == cell -> end () );
  }
}

void CellHashTest::testPathType () {
  // Test both KEEP and SWAP nodes in the same hash, accessed using
  // lower_bound and next.
  Permute::ConstPathRef
    keep = Permute::Path::connect (Permute::Path::epsilon (1, 7, 1.0f),
				   Permute::Path::epsilon (7, 11, 2.0f),
				   0.0f, false),
    swap = Permute::Path::connect (Permute::Path::epsilon (1, 5, 3.0f),
				   Permute::Path::epsilon (5, 11, 4.0f),
				   0.0f, true);
  CPPUNIT_ASSERT( cell -> empty () );
  CPPUNIT_ASSERT( ! cell -> insert (keep) );
  CPPUNIT_ASSERT( cell -> insert (keep) );
  CPPUNIT_ASSERT_EQUAL( size_t (1), cell -> size () );
  CPPUNIT_ASSERT( ! cell -> insert (swap) );
  CPPUNIT_ASSERT( cell -> insert (swap) );
  CPPUNIT_ASSERT_EQUAL( size_t (2), cell -> size () );
  // Verify KEEP node.
  Permute::CellHash::const_iterator k = cell -> lower_bound (1, Permute::Path::KEEP);
  CPPUNIT_ASSERT( k != cell -> end () );
  CPPUNIT_ASSERT( (* k) == keep );
  k = cell -> next (k, 1, Permute::Path::KEEP);
  CPPUNIT_ASSERT( k == cell -> end () );
  // Verify SWAP node.
  Permute::CellHash::const_iterator s = cell -> lower_bound (1, Permute::Path::SWAP);
  CPPUNIT_ASSERT( s != cell -> end () );
  CPPUNIT_ASSERT( (* s) == swap );
  s = cell -> next (s, 1, Permute::Path::SWAP);
  CPPUNIT_ASSERT( s == cell -> end () );
}
