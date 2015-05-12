#ifndef _PERMUTE_FEATURE_MAP_TEST_HH
#define _PERMUTE_FEATURE_MAP_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <PV.hh>

class FeatureMapTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( FeatureMapTest );
  CPPUNIT_TEST( testSetAll );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::FeatureMap * map_;
  Permute::FeatureType * pos_, * dist_, * word_;
  std::vector <Permute::FeatureType *> types_;
public:
  void setUp ();
  void tearDown ();

  void testSetAll ();
};

#endif//_PERMUTE_FEATURE_MAP_TEST_HH
