#include <algorithm>
#include <numeric>
#include "GradientChartTest.hh"
#include <Log.hh>
#include <ParseController.hh>

CPPUNIT_TEST_SUITE_REGISTRATION( GradientChartTest );

using namespace Permute;

void GradientChartTest::testGradients () {
  // Creates Permutation.
  Permutation pi;
  integerPermutation (pi, 3);
  // Creates GradientChart.
  GradientChart chart (pi);
  // Creates weights;
  WRef one (1.0), two (2.0), three (3.0);
  // Creates SumBeforeCostRef.
  SumBeforeCostRef sbc (new SumBeforeCost (3, "GradientChartTest::testGradients"));
  (* sbc) (0, 1) += one;
  (* sbc) (0, 2) += two;
  (* sbc) (1, 2) += three;
  // Creates PV.  Note nonstandard use.
  PV pv;
  pv ["one"] = one;
  pv ["two"] = two;
  pv ["three"] = three;
  // Creates GradientScorer.
  GradientScorer scorer (sbc, pi);
  // Creates ParseController.
  ParseControllerRef pc (CubicParseController::create ());
  // Parses the identity permutation to compute the gradients.
  chart.parse (pc, scorer, pv);
  // Checks the gradients of the three parameters.
  std::vector <double> scores (6);
  scores [0] = 6;
  scores [1] = scores [3] = 3;
  scores [2] = 5;
  scores [4] = 1;
  double (* the_exp) (double) = std::exp;
  std::transform (scores.begin (), scores.end (),
		  scores.begin (),
		  the_exp);
  double Z = std::accumulate (scores.begin (), scores.end (), 0.0);
  double first_num = std::exp (6) + std::exp (3) + std::exp (1);
  double second_num = std::exp (6) + std::exp (5) + std::exp (3);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 595.7313, Z, 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1 - first_num / Z, one.operator double (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1 - second_num / Z, two.operator double (), 1e-4 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1 - second_num / Z, three.operator double (), 1e-4 );
}
