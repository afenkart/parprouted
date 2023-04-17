#include <catch2/catch.hpp>

#include "parprouted.h"

namespace {

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {

  CHECK(arptab == nullptr); // global list head

  SECTION("parseproc") { parseproc(); }
}

} // namespace
