#include "PVTest.hh"
#include <Application.hh>

CPPUNIT_TEST_SUITE_REGISTRATION( PVTest );

class PVTestApp : Permute::Application {
public:
  PVTestApp () : Permute::Application ("PVTest") {}
  int main (const std::vector <std::string> & args) {
    return EXIT_SUCCESS;
  }
} app;

using namespace Permute;

void PVTest::setUp () {
  if (! readFile (pv_, "/export/grid/royt/search-perceptron/dependency-2.0.xml")) {
    CPPUNIT_FAIL( "Could not read PV." );
  }
}

void PVTest::tearDown () {

}

void PVTest::testTemplates () {
  CPPUNIT_FAIL( "Not implemented!" );
}

void PVTest::testDistances () {
  CPPUNIT_ASSERT_EQUAL( std::string ("1"), pv_.distance (1) );
  CPPUNIT_ASSERT_EQUAL( std::string ("2"), pv_.distance (2) );
  CPPUNIT_ASSERT_EQUAL( std::string ("3"), pv_.distance (3) );
  CPPUNIT_ASSERT_EQUAL( std::string ("4"), pv_.distance (4) );
  CPPUNIT_ASSERT_EQUAL( std::string ("5"), pv_.distance (5) );
  CPPUNIT_ASSERT_EQUAL( std::string (">5"), pv_.distance (6) );
  CPPUNIT_ASSERT_EQUAL( std::string (">5"), pv_.distance (10) );
  CPPUNIT_ASSERT_EQUAL( std::string (">10"), pv_.distance (11) );
}

void PVTest::testCopy () {
  CPPUNIT_FAIL( "Not implemented!" );
}

void PVTest::testPrefix () {
  CPPUNIT_ASSERT_EQUAL( 5, PV::PREFIX_SIZE );
  CPPUNIT_ASSERT_EQUAL( std::string ("four"), PV::prefix ("four") );
  CPPUNIT_ASSERT_EQUAL( std::string ("longe"), PV::prefix ("longer") );
}

void PVTest::testSuffix () {
  CPPUNIT_ASSERT_EQUAL( 5, PV::SUFFIX_SIZE );
  CPPUNIT_ASSERT_EQUAL( std::string ("four"), PV::suffix ("four") );
  CPPUNIT_ASSERT_EQUAL( std::string ("onger"), PV::suffix ("longer") );
}
