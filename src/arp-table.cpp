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

#include <algorithm>
#include <vector>

#include "parprouted.h" // arptab_entry

namespace {

class ArpTableImpl : public ArpTable {
  int findentry(struct in_addr ipaddr) const override {

    auto it = std::find_if(std::cbegin(arptab), std::cend(arptab),
                           [&ipaddr](const auto &elt) -> bool { return ipaddr == elt.ipaddr_ia; });

    if (it == std::cend(arptab))
      return 0;
    else
      return 1;
  }

  arptab_entry *replace_entry(struct in_addr ipaddr, const char *dev) override {

    auto it = std::find_if(std::begin(arptab), std::end(arptab), [&ipaddr, dev](auto &elt) {
      return ipaddr.s_addr == elt.ipaddr_ia.s_addr && strncmp(elt.ifname, dev, strlen(dev)) == 0;
    });

    if (it != std::cend(arptab)) {
      return &*it;
    }

    if (debug)
      printf("Creating new arptab entry %s(%s)\n", inet_ntoa(ipaddr), dev);

    arptab.push_back({.want_route = 1});

    return &arptab.back();
  }

  /* Remove all entires in arptab where ipaddr is NOT on interface dev */
  int remove_other_routes(struct in_addr ipaddr, const char *dev) override {
    int removed = 0;

    auto match = [&ipaddr, dev](const arptab_entry &elt) {
      return ipaddr== elt.ipaddr_ia && strcmp(dev, elt.ifname) != 0;
    };

    auto it = std::find_if(std::begin(arptab), std::end(arptab), match);

    if (it != std::cend(arptab)) {
      if (debug && it->want_route) {
        printf("Marking entry %s(%s) for removal\n", inet_ntoa(ipaddr), it->ifname);
      }
      it->want_route = 0;
      ++removed;
    }
    return removed;
  }

  void apply(std::function<void(const arptab_entry &)> func) {
    for (auto &entry : arptab) {
      func(entry);
    }
  }
};

} // namespace

std::unique_ptr<ArpTable> makeArpTable() { return std::make_unique<ArpTableImpl>(); }
