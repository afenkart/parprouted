#include <catch2/catch_all.hpp>

#include "fs-mock.h"
#include "parprouted.h"

namespace {

using trompeloeil::_;

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {
  FileSystemMock fileSystem{};
  SECTION("parseproc") {
    REQUIRE_CALL(fileSystem, fopen(_, _))
        .RETURN(reinterpret_cast<FILE *>(0xdeadbeef));
    REQUIRE_CALL(fileSystem, feof(_)).RETURN(false);
    parseproc(fileSystem);
  }
}

} // namespace
