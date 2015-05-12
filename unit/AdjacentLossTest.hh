#ifndef _PERMUTE_ADJACENT_LOSS_TEST_HH
#define _PERMUTE_ADJACENT_LOSS_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <AdjacentLoss.hh>

class AdjacentLossTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( AdjacentLossTest );
  CPPUNIT_TEST( testIdentity );
  CPPUNIT_TEST( testSame );
  CPPUNIT_TEST( testDifferent );
  CPPUNIT_TEST_SUITE_END();
public:
  void testIdentity ();
  void testSame ();
  void testDifferent ();
};

#endif//_PERMUTE_ADJACENT_LOSS_TEST_HH
