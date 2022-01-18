#include <catch2/catch.hpp>
#include "mister/mister.h"

unsigned int init_connect_packet() {
    packet_ctx *pctx;
    return mr_init_connect_packet(&pctx);
}

TEST_CASE( "Initialize CONNECT packet", "[connect]" ) {
    REQUIRE(init_connect_packet() == 0);
}