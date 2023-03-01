#include <catch2/catch.hpp>

#include "fs-mock.h"
#include "parprouted.h"

namespace {

using trompeloeil::_;
using namespace trompeloeil;

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {

  CHECK(arptab == nullptr); // global list head
  FileSystemMock fileSystem{};

  SECTION("parseproc") {
    trompeloeil::sequence seq;

    REQUIRE_CALL(fileSystem, fopen(_, _))
        .RETURN(reinterpret_cast<FILE *>(0xdeadbeef));
    REQUIRE_CALL(fileSystem, feof(_)).RETURN(false).IN_SEQUENCE(seq);
    REQUIRE_CALL(fileSystem, fgets(_, _, _))
        .LR_SIDE_EFFECT(
            auto constexpr line =
                R"(IP address       HW type     Flags       HW address            Mask     Device)";
            std::strcpy(_1, line))
        .RETURN(_1)
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(fileSystem, feof(_)).RETURN(false).IN_SEQUENCE(seq);
    REQUIRE_CALL(fileSystem, fgets(_, _, _))
        .LR_SIDE_EFFECT(
            auto constexpr line =
                R"(192.168.11.182   0x1         0x2         00:1e:74:00:4a:88     *        wlp58s0)";
            std::strcpy(_1, line))
        .RETURN(_1)
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(fileSystem, feof(_)).RETURN(true).IN_SEQUENCE(seq);
    REQUIRE_CALL(fileSystem, fclose(_)).RETURN(0);
    parseproc(fileSystem);
  }
}

} // namespace
