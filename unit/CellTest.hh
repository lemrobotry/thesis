#ifndef _PERMUTE_CELL_TEST_HH
#define _PERMUTE_CELL_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <Cell.hh>
#include <BestCell.hh>
#include <FullCell.hh>

class CellTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( CellTest );
  CPPUNIT_TEST( testEmpty );
  CPPUNIT_TEST( testClear );
  CPPUNIT_TEST( testBegin );
  CPPUNIT_TEST( testBegin1 );
  CPPUNIT_TEST( testFind );
//   CPPUNIT_TEST( testRBegin );
//   CPPUNIT_TEST( testRBegin1 );
  CPPUNIT_TEST( testAdd );
  CPPUNIT_TEST( testBuild );
//   CPPUNIT_TEST( testNewBuild );
  CPPUNIT_TEST( testGetBestPath );
//  CPPUNIT_TEST( stressTest );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::CellRef best, full;
  Permute::ConstPathRef arc;
public:
  void setUp ();
  void tearDown ();
  void addArc ();
  typedef void (* BuildFunction) (Permute::CellRef, Permute::ConstCellRef, Permute::ConstCellRef, double, bool, Permute::Path::Type);
  void testBuildFunction (BuildFunction);

  void testEmpty ();
  void testClear ();
  void testBegin ();
  void testBegin1 ();
  void testFind ();
  void testRBegin ();
  void testRBegin1 ();
  void testAdd ();
  void testBuild ();
  void testNewBuild ();
  void testGetBestPath ();
  void stressTest ();
};

#endif//_PERMUTE_CELL_TEST_HH
