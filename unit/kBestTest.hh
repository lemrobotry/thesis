#ifndef _PERMUTE_K_BEST_TEST_HH
#define _PERMUTE_K_BEST_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <Permutation.hh>

class kBestTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( kBestTest );
  CPPUNIT_TEST( testSchroeder );
  CPPUNIT_TEST( testEqualPath );
  CPPUNIT_TEST( testKBest );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::Permutation pi_;
  std::vector <int> schroeder_;
public:
  void setUp ();
  void tearDown ();

  void testSchroeder ();
  void testEqualPath ();
  void testKBest ();
};

#endif//_PERMUTE_K_BEST_TEST_HH
