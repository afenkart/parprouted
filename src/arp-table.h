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

#pragma once

#include <netinet/in.h>

#include <functional>
#include <memory>

constexpr auto ARP_TABLE_ENTRY_LEN = 20;

inline bool operator==(const in_addr &left, const in_addr &right) {
  return left.s_addr == right.s_addr;
}

struct arptab_entry {
  struct in_addr ipaddr_ia;
  char hwaddr[ARP_TABLE_ENTRY_LEN];
  char ifname[ARP_TABLE_ENTRY_LEN];
  time_t tstamp;
  int route_added;
  int incomplete;
  int want_route;

  friend auto operator<=>(const arptab_entry &, const arptab_entry &) = default;
};

extern std::vector<arptab_entry> arptab;

struct ArpTable {
  virtual int findentry(struct in_addr ipaddr) const = 0;
  virtual arptab_entry *replace_entry(struct in_addr ipaddr, const char *dev) = 0;
  virtual int remove_other_routes(struct in_addr ipaddr, const char *dev) = 0;
  virtual void apply(std::function<void(const arptab_entry &)>) = 0;
};

std::unique_ptr<ArpTable> makeArpTable();
