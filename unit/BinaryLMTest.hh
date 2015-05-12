#ifndef _PERMUTE_BINARY_LM_TEST_HH
#define _PERMUTE_BINARY_LM_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

class BinaryLMTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( BinaryLMTest );
  CPPUNIT_TEST( testGetState );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp ();
  void testGetState ();
  void tearDown ();
};

#endif//_PERMUTE_BINARY_LM_TEST_HH
