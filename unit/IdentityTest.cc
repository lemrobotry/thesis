#include <BeforeScorer.hh>

#include "IdentityTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( IdentityTest );

void IdentityTest::setUp () {
  controller_ = Permute::CubicParseController::create ();
  factory_ = Permute::ChartFactory::create ();
}
void IdentityTest::tearDown () {

}

// Test that when 0 1 2 and 0 2 1 or 1 0 2 have identical scores, 0 1 2 wins.
void IdentityTest::testIdentity3 () {
  size_t N = 3;
  std::istringstream in ("one two three");
  Permute::readPermutationWithAlphabet (pi_, in);
  // Test 0 2 1
  Permute::BeforeCost * bc = new Permute::BeforeCost (N);
  Permute::BeforeCostRef bcr (bc);
  bc -> setCost (0, 1, 1.0);
  bc -> setCost (0, 2, 1.0);
  Permute::ScorerRef scorer (new Permute::BeforeScorer (bcr, pi_));

  Permute::ChartRef chart = factory_ -> chart (pi_);
  Permute::Chart::permute (chart, controller_, scorer);
  pi_.reorder (chart -> getBestPath ());
  for (size_t i = 0; i < N; ++ i) {
    CPPUNIT_ASSERT_EQUAL( i, pi_ [i] );
  }
  // Test 1 0 2
  bc -> setCost (0, 1, 0.0);
  bc -> setCost (1, 2, 1.0);

  Permute::Chart::permute (chart, controller_, scorer);
  pi_.reorder (chart -> getBestPath ());
  for (size_t i = 0; i < N; ++ i) {
    CPPUNIT_ASSERT_EQUAL( i, pi_ [i] );
  }
}

// Test that when 0 1 2 3 4 and 0 2 3 1 4 have identical scores, 0 1 2 3 4 wins.
void IdentityTest::testIdentity5 () {
  size_t N = 5;
  std::istringstream in ("one two three four five");
  Permute::readPermutationWithAlphabet (pi_, in);
  // Test 0 2 3 1 4
  Permute::BeforeCost * bc = new Permute::BeforeCost (N);
  Permute::BeforeCostRef bcr (bc);
  for (int j = 0; j < (N - 1); ++ j) {
    bc -> setCost (0, j + 1, 1.0);
    bc -> setCost (j, N - 1, 1.0);
  }
  bc -> setCost (2, 3, 3.0);
  // Make 1 want to be either before 2 or after 3.
  bc -> setCost (1, 2, 1.0);
  bc -> setCost (3, 1, 1.0);
  Permute::ScorerRef scorer (new Permute::BeforeScorer (bcr, pi_));

  Permute::ChartRef chart = factory_ -> chart (pi_);
  Permute::Chart::permute  (chart, controller_, scorer);
  pi_.reorder (chart -> getBestPath ());
  for (size_t i = 0; i < N; ++ i) {
    CPPUNIT_ASSERT_EQUAL( i, pi_ [i] );
  }
}
