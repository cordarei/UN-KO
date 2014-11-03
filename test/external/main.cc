#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

TEST_CASE( "testing tests", "[temporary]" ) {
  REQUIRE( 1 == 1 );
}
