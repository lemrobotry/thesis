#include "FeatureTypeTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( FeatureTypeTest );

void FeatureTypeTest::setUp () {
  one = new Permute::FeatureType ("one", 10);
  two = new Permute::FeatureType ("two", 1000);
}

void FeatureTypeTest::tearDown () {
  delete one;
  delete two;
}

void FeatureTypeTest::testOneByte () {
  std::string compressed (1, char (0));
  
  CPPUNIT_ASSERT_EQUAL(compressed, one -> intern ("one"));
  
  compressed[0] = char (1);
  CPPUNIT_ASSERT_EQUAL(compressed, one -> intern ("two"));

  compressed[0] = char (0);
  CPPUNIT_ASSERT_EQUAL(compressed, one -> intern ("one"));

  compressed[0] = char (1);
  CPPUNIT_ASSERT_EQUAL(compressed, one -> intern ("two"));

  compressed[0] = char (2);
  CPPUNIT_ASSERT_EQUAL(compressed, one -> intern ("three"));
}

void FeatureTypeTest::testTwoBytes () {
  std::string compressed (2, char (0));

  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("one"));

  compressed[0] = char (1);
  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("two"));

  compressed[0] = char (0);
  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("one"));

  compressed[0] = char (1);
  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("two"));

  compressed[0] = char (2);
  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("three"));

  for (int i = 3; i < (1 << 8); ++ i) {
    std::stringstream str;
    str << i;
    two -> intern (str.str ());
  }

  compressed[0] = char (0);
  compressed[1] = char (1);
  CPPUNIT_ASSERT_EQUAL(compressed, two -> intern ("four"));
}

void FeatureTypeTest::testUnintern () {
  std::string s1 ("one");
  std::string s2 ("two");

  std::string z1 = one -> intern (s1);
  std::string z2 = one -> intern (s2);

  CPPUNIT_ASSERT_EQUAL( s1, one -> unintern (z1) );
  CPPUNIT_ASSERT_EQUAL( s2, one -> unintern (z2) );
}
