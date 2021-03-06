/*!
  \file gameteTest.cc
  \ingroup unit
  \brief Tests construction and assigment to KTfwd::gamete via std::move
*/

#include <boost/test/unit_test.hpp>
#include <fwdpp/forward_types.hpp>
#include <config.h>

using mut = KTfwd::mutation;
using gtype = KTfwd::gamete;

BOOST_AUTO_TEST_SUITE( test_gamete_base )

BOOST_AUTO_TEST_CASE( move_construct )
{
  //Neutral mutations at positions 0.1 and 0.9, resp.
  gtype g1(1),g2(1);
  std::vector<mut> mvector(1,mut(0.1,0.));

  g1.mutations.push_back(0);
  mvector.emplace_back(0.9,0.);
  g2.mutations.push_back(1);
  gtype g3( std::move(g2) );
  BOOST_CHECK_EQUAL( g1.mutations.size(), 1 );
  BOOST_CHECK_EQUAL( g2.mutations.size(), 0 );
  BOOST_CHECK_EQUAL( g3.mutations.size(), 1 );
}


BOOST_AUTO_TEST_CASE( move_assign )
{
  //Neutral mutations at positions 0.1 and 0.9, resp.
  gtype g1(1),g2(1);
  std::vector<mut> mvector(1,mut(0.1,0.));

  g1.mutations.push_back(0);
  mvector.emplace_back(0.9,0.);
  g2.mutations.push_back(1);

  //let's put in a selected mutation
  mvector.emplace_back(1.1,-0.2);
  g2.smutations.emplace_back(1);
  gtype g3 = std::move(g2);
  
  BOOST_CHECK_EQUAL( g1.mutations.size(), 1 );
  BOOST_CHECK_EQUAL( g2.mutations.size(), 0 );
  BOOST_CHECK_EQUAL( g2.smutations.size(), 0 );
  BOOST_CHECK_EQUAL( g3.mutations.size(), 1 );
  BOOST_CHECK_EQUAL( g3.smutations.size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
