#ifndef _PERMUTE_STATISTICS_TEST_HH
#define _PERMUTE_STATISTICS_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <Statistics.hh>

class StatisticsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( StatisticsTest );
  CPPUNIT_TEST( testRange );
  CPPUNIT_TEST( testEqual );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::Statistics stats_;
public:
  void setUp ();
  void tearDown ();

  void testRange ();
  void testEqual ();
};

#endif//_PERMUTE_STATISTICS_TEST_HH
