#include "parprouted.h"

#include "context-mock.h"

#include <catch2/catch.hpp>
#include <experimental/array>
#include <iostream>
#include <trompeloeil.hpp>

#include <linux/if_packet.h>

namespace {

using trompeloeil::_;
using namespace trompeloeil;
using namespace std::string_literals;

constexpr const char *TAGS = "arp";

template <typename T, size_t... Idx>
std::string hexdumpImpl(T buf[sizeof(Idx)], std::index_sequence<Idx...>) {
  std::stringstream out;
  out << std::hex << std::setfill('0');
  ((out << std::setw(2) << static_cast<int>(buf[Idx]) << ','), ...);
  std::string tmp = out.str();
  return tmp.substr(0, tmp.length() - 1);
}

template <typename T, size_t Nm> std::string hexdump(T (&buf)[Nm]) {
  return hexdumpImpl(buf, std::make_index_sequence<Nm>{});
}

TEST_CASE("arp-test", TAGS) {
  ContextMock context{};

  SECTION("arp_reply") {
    ether_arp arp_req{
        .ea_hdr = arphdr{htons(ARPHRD_ETHER), htons(ETH_P_IP), 6, 4, ARPOP_REQUEST},
        .arp_sha = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16},
        .arp_spa = {0x14, 0x13, 0x12, 0x11},
        .arp_tha = 0, // {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        .arp_tpa = {0x04, 0x03, 0x02, 0x01},
    };

    ether_arp_frame frame{ether_header{}, arp_req};

    struct sockaddr_ll ifs {                          //
      .sll_family = AF_PACKET,                        //
          .sll_protocol = htons(ETH_P_ARP),           //
          .sll_ifindex = 7,                           //
          .sll_hatype = ARPHRD_ETHER,                 //
          .sll_pkttype = PACKET_BROADCAST,            //
          .sll_halen = ETH_ALEN,                      //
          .sll_addr = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6}, //
    };

    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(11);
    REQUIRE_CALL(context, bind(11, _, sizeof(sockaddr_ll))).RETURN(0);

    REQUIRE_CALL(context, sendto(11, _, sizeof(ether_arp_frame), 0, _, sizeof(sockaddr_ll)))
        .RETURN(0);
    REQUIRE_CALL(context, close(11)).RETURN(0);
    arp_reply(&frame, &ifs, context);

    THEN("src/dst ip are swapped") {
      auto ether_hdr = frame.ether_hdr;
      auto arp_reply = frame.arp;

      INFO("arp_sha: " << hexdump(arp_reply.arp_sha));
      INFO("arp_sha: " << hexdump(arp_reply.arp_tha));

      CHECK(std::to_array(ether_hdr.ether_shost) ==
            std::experimental::make_array<uint8_t>(0x01, 0x02, 0x03, 0x04, 0x05, 0x06));
      CHECK(std::to_array(ether_hdr.ether_dhost) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16));

      CHECK(std::to_array(arp_reply.arp_sha) ==
            std::experimental::make_array<uint8_t>(0x01, 0x02, 0x03, 0x04, 0x05, 0x06));
      CHECK(std::to_array(arp_reply.arp_tha) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16));

      INFO("arp_spa: " << hexdump(arp_reply.arp_spa));
      INFO("arp_tpa: " << hexdump(arp_reply.arp_tpa));

      CHECK(std::to_array(arp_reply.arp_spa) ==
            std::experimental::make_array<uint8_t>(0x04, 0x03, 0x02, 0x01));
      CHECK(std::to_array(arp_reply.arp_tpa) ==
            std::experimental::make_array<uint8_t>(0x14, 0x13, 0x12, 0x11));
    }
  }

  SECTION("arp_req") {
    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(7);
    arp_req("eth0", in_addr{htonl(0x01020304)}, false, context);
  }
}

} // namespace
