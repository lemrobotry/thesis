#ifndef _PERMUTE_CELL_HASH_TEST_HH
#define _PERMUTE_CELL_HASH_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <CellHash.hh>

class CellHashTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( CellHashTest );
  CPPUNIT_TEST( testEmpty );
  CPPUNIT_TEST( testClear );
  CPPUNIT_TEST( testInsert );
  CPPUNIT_TEST( testFind );
  CPPUNIT_TEST( testErase );
  CPPUNIT_TEST( testIterator );
  CPPUNIT_TEST( testBoundIterator );
//   CPPUNIT_TEST( testRIterator );
//   CPPUNIT_TEST( testRBoundIterator );
//   CPPUNIT_TEST( stressTest );
//   CPPUNIT_TEST( stressTest2 );
  CPPUNIT_TEST( testPathType );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::CellHash * cell;
  Permute::ConstPathRef arc_5_12, arc_8_15;
public:
  void setUp ();
  void tearDown ();

  void testEmpty ();
  void testClear ();
  void testInsert ();
  void testFind ();
  void testErase ();
  void testIterator ();
  void testBoundIterator ();
  void testRIterator ();
  void testRBoundIterator ();
  void stressTest ();
  void stressTest2 ();
  void testPathType ();
};

#endif//_PERMUTE_CELL_HASH_TEST_HH
