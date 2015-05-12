#include "AdjacentGradientChartTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( AdjacentGradientChartTest );

using namespace Permute;

void AdjacentGradientChartTest::setUp () {
  // Creates Permutation.
  integerPermutation (pi, 3);

  chart = new AdjacentGradientChart (pi);

  WRef one (1.0), two (2.0), three (3.0);

  SumBeforeCostRef sbc (new SumBeforeCost (3, "AdjacentGradientChartTest::testExpectation"));
  (* sbc) (0, 1) += one;
  (* sbc) (0, 2) += two;
  (* sbc) (1, 2) += three;

  pv ["one"] = one;
  pv ["two"] = two;
  pv ["three"] = three;

  ExpectationGradientScorer scorer (sbc, pi);

  ParseControllerRef pc (CubicParseController::create ());

  chart -> parse (pc, scorer, pv);
}

void AdjacentGradientChartTest::tearDown () {
  delete chart;
}

void AdjacentGradientChartTest::testInside () {
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> inside (0, 1).any ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> inside (1, 2).any ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> inside (2, 3).any ().expectation (), 1e-4 );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> inside (0, 2).swap ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0, chart -> inside (0, 2).keep ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.7310586, chart -> inside (0, 2).any ().expectation (), 1e-4 );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> inside (1, 3).swap ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0, chart -> inside (1, 3).keep ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.9525741, chart -> inside (1, 3).any ().expectation (), 1e-4 );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.392677, chart -> inside (0, 3).any ().expectation (), 1e-4 );
}

void AdjacentGradientChartTest::testOutside () {
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (0, 3).keepSameRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (0, 3).keepNewRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (0, 3).swap ().expectation (), 1e-4 );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (1, 3).keepSameRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (1, 3).keepNewRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (1, 3).swap ().expectation (), 1e-4 );

  // 1 2 3: <5, 1> + 3 1 2: <0, 0>
  CPPUNIT_ASSERT_DOUBLES_EQUAL( std::exp (5) / (std::exp (5) + 1), chart -> outside (0, 2).keepSameRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (0, 2).keepNewRight ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, chart -> outside (0, 2).swap ().expectation (), 1e-4 );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.392677, chart -> outside (0, 1).leaf ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.392677, chart -> outside (1, 2).leaf ().expectation (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.392677, chart -> outside (2, 3).leaf ().expectation (), 1e-4 );
}

void AdjacentGradientChartTest::testGradient () {
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3625316, pv ["one"].operator double (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.01736893, pv ["two"].operator double (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0510847, pv ["three"].operator double (), 1e-4 );
}
