#ifndef _PERMUTE_GRADIENT_CHART_TEST_HH
#define _PERMUTE_GRADIENT_CHART_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <GradientChart.hh>

class GradientChartTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( GradientChartTest );
  CPPUNIT_TEST( testGradients );
  CPPUNIT_TEST_SUITE_END();
public:
  void testGradients ();
};

#endif//_PERMUTE_GRADIENT_CHART_TEST_HH
