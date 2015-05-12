#ifndef _PERMUTE_FEATURE_TYPE_TEST_HH
#define _PERMUTE_FEATURE_TYPE_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <PV.hh>

class FeatureTypeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( FeatureTypeTest );
  CPPUNIT_TEST( testOneByte );
  CPPUNIT_TEST( testTwoBytes );
  CPPUNIT_TEST( testUnintern );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::FeatureType * one, * two;
public:
  void setUp ();
  void tearDown ();

  void testOneByte ();
  void testTwoBytes ();
  void testUnintern ();
};

#endif//_PERMUTE_FEATURE_TYPE_TEST_HH
