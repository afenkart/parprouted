#include "parprouted.h"

#include "context-mock.h"

#include <catch2/catch.hpp>
#include <trompeloeil.hpp>

#include <net/ethernet.h>

namespace {

using trompeloeil::_;
using namespace trompeloeil;
using namespace std::string_literals;

constexpr const char *TAGS = "arp";

TEST_CASE("arp-test", TAGS) {
  ContextMock context{};

  SECTION("arp_reply") {
    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(11);
    arp_reply(nullptr, nullptr, context);
  }
}

} // namespace
