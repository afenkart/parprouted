#include "parprouted.h"

#include <catch2/catch.hpp>
#include <trompeloeil.hpp>

namespace {

using trompeloeil::_;
using namespace trompeloeil;
using namespace std::string_literals;

constexpr const char *TAGS = "arp";

TEST_CASE("arp-test", TAGS) {
  SECTION("arp_reply") { arp_reply(nullptr, nullptr); }
}

} // namespace
