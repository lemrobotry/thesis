#ifndef _PERMUTE_PV_TEST_HH
#define _PERMUTE_PV_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <PV.hh>

class PVTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( PVTest );
  CPPUNIT_TEST( testTemplates );
  CPPUNIT_TEST( testDistances );
  CPPUNIT_TEST( testCopy );
  CPPUNIT_TEST( testPrefix );
  CPPUNIT_TEST( testSuffix );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::PV pv_;
public:
  void setUp ();
  void tearDown ();
  void testTemplates ();
  void testDistances ();
  void testCopy ();
  void testPrefix ();
  void testSuffix ();
};

#endif//_PERMUTE_PV_TEST_HH
