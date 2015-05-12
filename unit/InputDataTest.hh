#ifndef _PERMUTE_INPUT_DATA_TEST_HH
#define _PERMUTE_INPUT_DATA_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <InputData.hh>

class InputDataTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( InputDataTest );
  CPPUNIT_TEST( testOneInput );
  CPPUNIT_TEST( testTwoInputs );
  CPPUNIT_TEST( testDependency );
  CPPUNIT_TEST_SUITE_END();
public:
  void testOneInput();
  void testTwoInputs();
  void testDependency();
};

#endif//_PERMUTE_INPUT_DATA_TEST_HH
