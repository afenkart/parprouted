#include <catch2/catch.hpp>

#include "fs-mock.h"
#include "parprouted.h"

namespace {

using trompeloeil::_;
using namespace trompeloeil;

constexpr const char *TAGS = "foo";

TEST_CASE("parprouted-test", TAGS) {
  // debug = verbose = true;

  [](auto &list) {
    while (list != nullptr) {
      free(std::exchange(list, list->next));
    }
  }(arptab);

  CHECK(arptab == nullptr);
  FileSystemMock fileSystem{};

  in_addr ip1{htonl(0x00000001)};
  in_addr ip2{htonl(0x00000002)};
  const char *dev0{"dev0"};
  const char *dev1{"dev1"};

  auto emptyCache = []() { return arptab == nullptr; };

  auto sizeCache = []() {
    int count{};
    for (const auto *cur = arptab; cur; cur = cur->next) {
      count++;
    }
    return count;
  };

  SECTION("arp table cache") {
    GIVEN("empty cache") {
      THEN("cache is empty") {
        CHECK(emptyCache());
        CHECK(sizeCache() == 0);
        CHECK(findentry(ip1) == 0);
        CHECK(findentry(ip2) == 0); // false
      }

      WHEN("replace_entry entry") {
        auto entry = replace_entry(ip1, dev0);
        CHECK(entry != nullptr);
        CHECK(arptab == entry);

        WHEN("not populating the new entry") {
          THEN("entry is not found by ip") { CHECK(findentry(ip1) == 0); }
          THEN("same entry is added again") {
            auto entry2 = replace_entry(ip1, dev0);
            CHECK(entry2 != entry);
          }
        }

        WHEN("populating the new entry") {
          strcpy(entry->ifname, dev0);
          entry->ipaddr_ia = ip1;

          THEN("same entry is not added") {
            auto entry2 = replace_entry(ip1, dev0);
            CHECK(entry2 == entry);
          }
          THEN("entry is found by ip") {
            CHECK(findentry(ip1) == 1);
            CHECK(findentry(ip2) == 0); // false
          }
          WHEN("same ip different interface") {
            auto entry2 = replace_entry(ip1, dev1);
            THEN("new entry is created") { CHECK(entry2 != entry); }

            THEN("entry is found by ip") { CHECK(findentry(ip1) == 1); }
          }
          WHEN("different ip same interface") {
            auto entry2 = replace_entry(ip2, dev0);
            THEN("new entry is created") { CHECK(entry2 != entry); }

            WHEN("populating new entry") {
              strcpy(entry2->ifname, dev0);
              entry2->ipaddr_ia = ip2;

              THEN("entry is found by ip") { CHECK(findentry(ip2) == 1); }
            }
          }
        }
      }
    }

    GIVEN("cache with 2 entries: ip1@dev0, ip1@dev1") {
      auto createEntry = [](auto &&ip, auto &&dev) {
        auto entry = replace_entry(ip, dev);
        strcpy(entry->ifname, dev);
        entry->ipaddr_ia = ip;
        entry->tstamp = time(NULL);
        return entry;
      };
      [[maybe_unused]] auto entry1 = createEntry(ip1, dev0);
      [[maybe_unused]] auto entry2 = createEntry(ip1, dev1);

      THEN("ip is present") {
        CHECK(findentry(ip1) == 1);
        CHECK(!emptyCache());
        CHECK(sizeCache() == 2);
      }
      WHEN("remove_other_routes for non dev0") {
        debug = verbose = true;
        CHECK(remove_other_routes(ip1, dev0) == 1);
        THEN("entries only marked for removal") {
          CHECK(entry1->want_route == true);
          CHECK(entry2->want_route == false);
          CHECK(sizeCache() == 2); // nothing actually removed yet
        }
        WHEN("processarp") {
          processarp(false);
          THEN("entry is removed") {
            CHECK(sizeCache() == 1);
            CHECK(replace_entry(ip1, dev0) == entry1);
          }
        }
      }
      WHEN("remove_other_routes for non dev3") {
        CHECK(remove_other_routes(ip1, "dev3") == 2);
        WHEN("processarp") {
          processarp(false);
          THEN("both entries removed") { CHECK(emptyCache()); }
        }
      }
    }

    GIVEN("2 expired entries") {
      auto now = time(NULL);
      auto createExpiredEntry = [now](auto &&ip, auto &&dev) {
        auto entry = replace_entry(ip, dev);
        strcpy(entry->ifname, dev);
        entry->ipaddr_ia = ip;
        entry->tstamp = now - 2 * ARP_TABLE_ENTRY_TIMEOUT;
        return entry;
      };
      [[maybe_unused]] auto entry1 = createExpiredEntry(ip1, dev0);
      [[maybe_unused]] auto entry2 = createExpiredEntry(ip1, dev1);

      WHEN("processarp") {
        processarp(false);
        THEN("cache is empty") { CHECK(emptyCache()); }
      }
    }
  }

  SECTION("parseproc") {
    trompeloeil::sequence seq;

    REQUIRE_CALL(fileSystem, fopen(_, _)).RETURN(reinterpret_cast<FILE *>(0xdeadbeef));
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
