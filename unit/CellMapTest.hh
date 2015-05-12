#ifndef _PERMUTE_CELL_MAP_TEST_HH
#define _PERMUTE_CELL_MAP_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <CellMap.hh>

class CellMapTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( CellMapTest );
  CPPUNIT_TEST( testFsaCellMap );
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp ();
  void tearDown ();

  void testFsaCellMap ();
};

#endif//_PERMUTE_CELL_MAP_TEST_HH
