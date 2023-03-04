/* parprouted: ProxyARP routing daemon.
 * (C) 2008 Vladimir Ivaschenko <vi@maks.net>
 * (C) 2023 Andreas Fenkart <afenkart@gmail.com>
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "arp-table.h"

#include <catch2/catch_all.hpp>

namespace {

constexpr const char *TAGS = "arptable";

TEST_CASE("arptable", TAGS) {
  auto arpTable_ = makeArpTable();
  auto &arpTable = *arpTable_;

  in_addr inAddr1{0x12171819};
  const char dev0[]{"dev0"};

  SECTION("add/find") {
    WHEN("empty") {
      CHECK(!arpTable.findentry(inAddr1));
      WHEN("create new entry") {
        auto &entry = *arpTable.replace_entry(inAddr1, dev0);
        THEN("fields are not populated") { CHECK(entry == arptab_entry{.want_route = 1}); }
        AND_THEN("entry contains no addresss or data") { CHECK(!arpTable.findentry(inAddr1)); }
        WHEN("address is populated") {
          entry.ipaddr_ia = inAddr1;
          THEN("entry is found") { CHECK(arpTable.findentry(inAddr1)); }
        }
      }
    }
  }
  SECTION("remove other routes") {}
}

} // namespace
