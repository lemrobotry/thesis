#ifndef _PERMUTE_ADJACENT_GRADIENT_CHART_TEST_HH
#define _PERMUTE_ADJACENT_GRADIENT_CHART_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <AdjacentLoss.hh>

class AdjacentGradientChartTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( AdjacentGradientChartTest );
  CPPUNIT_TEST( testInside );
  CPPUNIT_TEST( testOutside );
  CPPUNIT_TEST( testGradient );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::Permutation pi;
  Permute::AdjacentGradientChart * chart;
  Permute::PV pv;
public:
  void setUp ();
  void tearDown ();
  void testInside ();
  void testOutside ();
  void testGradient ();
};

#endif//_PERMUTE_ADJACENT_GRADIENT_CHART_TEST_HH
