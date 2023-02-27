#include <catch2/catch_all.hpp>

#include "parprouted.h"

namespace {

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {

  arptab_entry *arptab_head = nullptr;
  arptab = &arptab_head; // global list head

  SECTION("parseproc") { parseproc(); }
}

} // namespace
