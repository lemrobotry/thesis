#include "ExpectationTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( ExpectationTest );

using namespace Permute;

void ExpectationTest::testSum () {
  Expectation e = Expectation::p_pv (3.0, Log::One);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0, e.expectation (), 1e-4 );

  e += Expectation::Zero;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0, e.expectation (), 1e-4 );

  e += Expectation::One;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( std::exp (3) / (std::exp (3) + 1), e.expectation (), 1e-4 );
}

void ExpectationTest::testProduct () {
  Expectation e = Expectation::One + Expectation::p_pv (1.0, Log::One);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( std::exp (1) / (std::exp (1) + 1), e.expectation (), 1e-4 );

  e *= Expectation::One;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( std::exp (1) / (std::exp (1) + 1), e.expectation (), 1e-4 );

  e *= Expectation::p_pv (4.0, Log::One);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( (2 * std::exp (5) + std::exp (4)) / (std::exp (5) + std::exp (4)), e.expectation (), 1e-4 );

  e *= Expectation::Zero;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, e.expectation (), 1e-4 );
}
