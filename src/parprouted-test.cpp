#include <catch2/catch_all.hpp>

#include "fs-mock.h"
#include "parprouted.h"

namespace {

using trompeloeil::_;
using namespace trompeloeil;

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {
  FileSystemMock fileSystem{};
  SECTION("parseproc") {
    REQUIRE_CALL(fileSystem, fopen(_, _))
        .RETURN(reinterpret_cast<FILE *>(0xdeadbeef));
    REQUIRE_CALL(fileSystem, feof(_)).RETURN(false);
    REQUIRE_CALL(fileSystem, fgets(_, _, _))
        .LR_SIDE_EFFECT(
            auto constexpr firstline =
                R"(IP address       HW type     Flags       HW address            Mask     Device)";
            std::strcpy(_1, firstline))
        .RETURN(_1);
    parseproc(fileSystem);
#if 0
    192.168.11.182   0x1         0x2         00:1e:74:00:4a:88     *        wlp58s0
#endif
  }
}

} // namespace
