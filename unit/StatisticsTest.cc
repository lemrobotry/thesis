#include "StatisticsTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( StatisticsTest );

void StatisticsTest::setUp () {
  stats_.reset ();
}

void StatisticsTest::tearDown () {
}

void StatisticsTest::testRange () {
  for (int i = 0; i < 100; ++ i) {
    std::ostringstream message;
    message << i;
    stats_ += i;
    CPPUNIT_ASSERT_EQUAL_MESSAGE( message.str (), i + 1, stats_.n () );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( message.str (), i / 2.0, stats_.mean () );
    if (i == 0) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( message.str (), 0.0, stats_.variance () );
    } else {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( message.str (), (i + 1) * (i + 2) / 12.0, stats_.variance () );
    }
  }
}

void StatisticsTest::testEqual () {
  for (int i = 0; i < 100; ++ i) {
    std::ostringstream message;
    message << i;
    stats_ += 3.14159265;
    CPPUNIT_ASSERT_EQUAL_MESSAGE( message.str (), i + 1, stats_.n () );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3.14159265, stats_.mean (), 1e-10 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, stats_.variance (), 1e-10 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, stats_.stdev (), 1e-5 );
  }
}
