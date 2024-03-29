#include "parprouted.h"

#include "context-mock.h"

#include <catch2/catch.hpp>
#include <experimental/array>
#include <iostream>
#include <trompeloeil.hpp>

#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>

namespace {

using trompeloeil::_;
using namespace trompeloeil;
using namespace std::string_literals;

constexpr const char *TAGS = "arp";

template <typename T, size_t Nm, size_t... Idx>
constexpr std::array<T, sizeof...(Idx)> makeSubArrayImpl(T (&buf)[Nm],
                                                         std::index_sequence<Idx...>) {
  return {{buf[Idx]...}};
}

template <size_t firstN, typename T, size_t Nm>
constexpr std::array<T, firstN> makeSubArray(T (&buf)[Nm]) {
  return makeSubArrayImpl(buf, std::make_index_sequence<firstN>{});
}

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

  SECTION("arp_req for ip 1.2.3.4") {
    REQUIRE_CALL(context, socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))).RETURN(7);
    REQUIRE_CALL(context, ioctl3(7, SIOCGIFHWADDR, _))
        .LR_SIDE_EFFECT(
            auto hwaddr = std::experimental::make_array<char>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
            memcpy(static_cast<ifreq *>(_3)->ifr_hwaddr.sa_data, hwaddr.data(), hwaddr.size()))
        .RETURN(0);
    REQUIRE_CALL(context, ioctl3(7, SIOCGIFINDEX, _))
        .LR_SIDE_EFFECT(static_cast<ifreq *>(_3)->ifr_ifindex = 2)
        .RETURN(0);
    REQUIRE_CALL(context, ioctl3(7, SIOCGIFADDR, _))
        .LR_SIDE_EFFECT(struct sockaddr_in ip{
            .sin_family = 0, .sin_port = 0, .sin_addr = htonl(0x11121314), .sin_zero{0x0}};
                        memcpy(&static_cast<ifreq *>(_3)->ifr_addr, &ip, sizeof(ip)))
        .RETURN(0);

    ether_arp_frame packet{};
    sockaddr_ll ifs{};

    REQUIRE_CALL(context, sendto(7, _, sizeof(ether_arp_frame), 0, _, sizeof(sockaddr_ll)))
        .LR_SIDE_EFFECT(packet = *static_cast<const ether_arp_frame *>(_2))
        .LR_SIDE_EFFECT(ifs = *reinterpret_cast<const sockaddr_ll *>(_5))
        .RETURN(0);
    REQUIRE_CALL(context, close(7)).RETURN(0);

    arp_req("eth0", in_addr{htonl(0x01020304)}, false, context);

    THEN("arp packet is created") {

      CHECK(makeSubArray<6>(ifs.sll_addr) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16));
      CHECK(ifs.sll_family == AF_PACKET);
      CHECK(ifs.sll_protocol == htons(ETH_P_ARP));
      CHECK(ifs.sll_ifindex == 2); // see ioctl(SIOCGIFINDEX) above
      CHECK(ifs.sll_hatype == ARPHRD_ETHER);
      CHECK(ifs.sll_pkttype == PACKET_BROADCAST);
      CHECK(ifs.sll_halen == ETH_ALEN);

      ether_header etherHeader = packet.ether_hdr;
      CHECK(std::to_array(etherHeader.ether_dhost) ==
            std::experimental::make_array<uint8_t>(0xff, 0xff, 0xff, 0xff, 0xff, 0xff));
      CHECK(std::to_array(etherHeader.ether_shost) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16));
      CHECK(etherHeader.ether_type == htons(ETHERTYPE_ARP));

      ether_arp arpFrame = packet.arp;
      CHECK(arpFrame.arp_hrd == htons(ARPHRD_ETHER));
      CHECK(arpFrame.arp_pro == htons(ETH_P_IP));
      CHECK(arpFrame.arp_hln == ETH_ALEN);
      CHECK(arpFrame.arp_pln == 4);

      CHECK(std::to_array(arpFrame.arp_tha) == std::array<uint8_t, ETH_ALEN>());
      CHECK(std::to_array(arpFrame.arp_sha) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14, 0x15, 0x16));

      // TODO gratuitous
      CHECK(std::to_array(arpFrame.arp_tpa) ==
            std::experimental::make_array<uint8_t>(0x01, 0x02, 0x03, 0x04));
      CHECK(std::to_array(arpFrame.arp_spa) ==
            std::experimental::make_array<uint8_t>(0x11, 0x12, 0x13, 0x14));
      CHECK(arpFrame.arp_op == htons(ARPOP_REQUEST));
    }
  }
}

} // namespace
