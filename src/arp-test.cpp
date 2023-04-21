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

    struct sockaddr_ll ifs {                //
      .sll_family = AF_PACKET,              //
          .sll_protocol = htons(ETH_P_ARP), //
          .sll_ifindex = 7,                 //
          .sll_hatype = ARPHRD_ETHER,       //
          .sll_pkttype = PACKET_BROADCAST,  //
          .sll_halen = ETH_ALEN,            //
          .sll_addr = {01, 02, 03, 04, 05, 06, 07, 0x08},
    };

    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(11);
    REQUIRE_CALL(context, bind(11, _, sizeof(sockaddr_ll))).RETURN(0);
    arp_reply(&frame, &ifs, context);
  }
}

} // namespace
