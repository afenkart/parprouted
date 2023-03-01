#include <catch2/catch_all.hpp>

#include "fs-mock.h"
#include "parprouted.h"

namespace {

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {
  FileSystemMock fileSystem{};
  SECTION("parseproc") { parseproc(fileSystem); }
}

} // namespace
