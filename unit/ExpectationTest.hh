#ifndef _PERMUTE_EXPECTATION_TEST_HH
#define _PERMUTE_EXPECTATION_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <ExpectationSemiring.hh>

class ExpectationTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( ExpectationTest );
  CPPUNIT_TEST( testSum );
  CPPUNIT_TEST( testProduct );
  CPPUNIT_TEST_SUITE_END();
public:
  void testSum ();
  void testProduct ();
  void testIncrease ();
  void testScale ();
};

#endif//_PERMUTE_EXPECTATION_TEST_HH
