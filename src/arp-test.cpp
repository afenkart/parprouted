#include "parprouted.h"

#include "context-mock.h"

#include <catch2/catch.hpp>
#include <trompeloeil.hpp>

#include <linux/if_packet.h>

namespace {

using trompeloeil::_;
using namespace trompeloeil;
using namespace std::string_literals;

constexpr const char *TAGS = "arp";

TEST_CASE("arp-test", TAGS) {
  ContextMock context{};

  SECTION("arp_reply") {
    ether_arp_frame frame{ether_header{}, ether_arp{}};
    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(11);
    REQUIRE_CALL(context, bind(11, _, sizeof(sockaddr_ll))).RETURN(0);
    arp_reply(&frame, nullptr, context);
  }
}

} // namespace
