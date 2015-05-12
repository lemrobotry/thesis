#ifndef _PERMUTE_IDENTITY_TEST_HH
#define _PERMUTE_IDENTITY_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <ChartFactory.hh>

// Ensure that the identity permutation is the best path if it ties
// with other permutations.
class IdentityTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( IdentityTest );
  CPPUNIT_TEST( testIdentity3 );
  CPPUNIT_TEST( testIdentity5 );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::ParseControllerRef controller_;
  Permute::ChartFactoryRef factory_;
  Permute::Permutation pi_;
public:
  void setUp ();
  void tearDown ();

  void testIdentity3 ();
  void testIdentity5 ();
};

#endif//_PERMUTE_IDENTITY_TEST_HH
