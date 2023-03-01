#include <catch2/catch_all.hpp>

#include "parprouted.h"

namespace {

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {
  SECTION("parseproc") { parseproc(); }
}

} // namespace
