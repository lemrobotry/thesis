#ifndef _PERMUTE_PATH_TEST_HH
#define _PERMUTE_PATH_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <Path.hh>

class PathTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( PathTest );
  CPPUNIT_TEST( testGetStart );
  CPPUNIT_TEST( testGetEnd );
  CPPUNIT_TEST( testGetScore );
  CPPUNIT_TEST( testNormal );
  CPPUNIT_TEST( testLessThan );
  CPPUNIT_TEST( testRLessThan );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::ConstPathRef null, lower, rlower, upper, rupper, forsearch,
    arc1, arc2, connect, swap, epsilon, final, addscore;
public:
  void setUp ();
  void tearDown ();

  void testGetStart ();
  void testGetEnd ();
  void testGetScore ();
  void testNormal ();
  void testLessThan ();
  void testRLessThan ();
};

#endif//_PERMUTE_PATH_TEST_HH
