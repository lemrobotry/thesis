#ifndef _PERMUTE_EPSILON_CLOSURE_TEST_HH
#define _PERMUTE_EPSILON_CLOSURE_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <EpsilonClosure.hh>

class EpsilonClosureTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( EpsilonClosureTest );
  CPPUNIT_TEST( testEpsilonClosure );
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp ();
  void tearDown ();

  void testEpsilonClosure ();
};

#endif//_PERMUTE_EPSILON_CLOSURE_TEST_HH
