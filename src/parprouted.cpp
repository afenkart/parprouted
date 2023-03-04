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
 *
 */

#include "parprouted.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "arp-table.h"
#include "fs.h"

int debug = 0;
int verbose = 0;
int option_arpperm = 0;

char *errstr;

char *ifaces[MAX_IFACES];
int last_iface_idx = -1;

std::vector<arptab_entry> arptab;
pthread_mutex_t arptab_mutex;

/* Remove route from kernel */
int route_remove(arptab_entry *cur_entry) {
  char routecmd_str[ROUTE_CMD_LEN];
  int success = 1;

  if (snprintf(routecmd_str, ROUTE_CMD_LEN - 1,
               "/sbin/ip route del %s/32 metric 50 dev %s scope link",
               inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN - 1) {
    syslog(LOG_INFO, "ip route command too large to fit in buffer!");
  } else {
    if (system(routecmd_str) != 0) {
      syslog(LOG_INFO, "'%s' unsuccessful!", routecmd_str);
      if (debug)
        printf("%s failed\n", routecmd_str);
      success = 0;
    } else {
      if (debug)
        printf("%s success\n", routecmd_str);
      success = 1;
    }
  }
  if (success)
    cur_entry->route_added = 0;

  return success;
}

/* Add route into kernel */
int route_add(arptab_entry *cur_entry) {
  char routecmd_str[ROUTE_CMD_LEN];
  int success = 1;

  if (snprintf(routecmd_str, ROUTE_CMD_LEN - 1,
               "/sbin/ip route add %s/32 metric 50 dev %s scope link",
               inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN - 1) {
    syslog(LOG_INFO, "ip route command too large to fit in buffer!");
  } else {
    if (system(routecmd_str) != 0) {
      syslog(LOG_INFO, "'%s' unsuccessful, will try to remove!", routecmd_str);
      if (debug)
        printf("%s failed\n", routecmd_str);
      route_remove(cur_entry);
      success = 0;
    } else {
      if (debug)
        printf("%s success\n", routecmd_str);
      success = 1;
    }
  }
  if (success)
    cur_entry->route_added = 1;

  return success;
}

void processarp(int in_cleanup) {
  std::vector<arptab_entry> expiredEntries{};

  auto expired = [in_cleanup](const arptab_entry &it) {
    return !it.want_route || time(NULL) - it.tstamp > ARP_TABLE_ENTRY_TIMEOUT || in_cleanup;
  };

  std::copy_if(std::cbegin(arptab), std::cend(arptab), std::back_inserter(expiredEntries), expired);

  for (auto &it : expiredEntries) {
    if (it.route_added)
      route_remove(&it);

    /* remove from arp list */
    if (debug)
      printf("Delete arp %s(%s)\n", inet_ntoa(it.ipaddr_ia), it.ifname);
  }

  arptab.erase(std::remove_if(std::begin(arptab), std::end(arptab), expired), arptab.end());

  /* Now loop to add new routes */
  for (auto &elt : arptab) {
    if (!expired(elt)) {
      route_add(&elt);
    }
  }
}

void parseproc(ArpTable &arpTable, FileSystem &fileSystem) {
  FILE *arpf;
  int firstline;
  arptab_entry *entry;
  char line[ARP_LINE_LEN];
  struct in_addr ipaddr;
  int incomplete = 0;
  int i;
  char *ip, *mac, *dev;

  /* Parse /proc/net/arp table */

  if ((arpf = fileSystem.fopen(PROC_ARP, "r")) == NULL) {
    errstr = strerror(errno);
    syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
  }

  firstline = 1;

  while (!fileSystem.feof(arpf)) {

    if (fileSystem.fgets(line, ARP_LINE_LEN, arpf) == NULL) {
      if (!fileSystem.ferror(arpf))
        break;
      else {
        errstr = strerror(errno);
        syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
      }
    } else {
      if (firstline) {
        firstline = 0;
        continue;
      }
      if (debug && verbose)
        printf("read ARP line %s", line);

      incomplete = 0;

      /* Incomplete ARP entries with MAC 00:00:00:00:00:00 */
      if (strstr(line, "00:00:00:00:00:00") != NULL)
        incomplete = 1;

      /* Incomplete entries having flag 0x0 */
      if (strstr(line, "0x0") != NULL)
        incomplete = 1;

      ip = strtok(line, " ");

      if ((inet_aton(ip, &ipaddr)) == -1)
        syslog(LOG_INFO, "Error parsing IP address %s", ip);

      /* if IP address is marked as undiscovered and does not exist in arptab,
         send ARP request to all ifaces */

      std::cerr << "incomplete " << incomplete << "\n";

      if (incomplete && arpTable.contains(ipaddr)) {
        if (debug)
          printf("incomplete entry %s found, request on all interfaces\n", inet_ntoa(ipaddr));
        for (i = 0; i <= last_iface_idx; i++)
          arp_req(ifaces[i], ipaddr, 0);
      }

      /* Hardware type */
      strtok(NULL, " ");

      /* flags */
      strtok(NULL, " ");

      /* MAC address */
      mac = strtok(NULL, " ");

      /* Mask */
      strtok(NULL, " ");

      /* Device */
      dev = strtok(NULL, " ");

      if (dev[strlen(dev) - 1] == '\n') {
        dev[strlen(dev) - 1] = '\0';
      }

      entry = arpTable.replace_entry(ipaddr, dev);

      std::cerr << "entry->incomplete " << entry->incomplete << "\n";

      if (entry->incomplete != incomplete && debug)
        printf("change entry %s(%s) to incomplete=%d\n", ip, dev, incomplete);

      entry->ipaddr_ia.s_addr = ipaddr.s_addr;
      entry->incomplete = incomplete;

      if (strlen(mac) < ARP_TABLE_ENTRY_LEN)
        strncpy(entry->hwaddr, mac, ARP_TABLE_ENTRY_LEN);
      else
        syslog(LOG_INFO, "Error during ARP table parsing");

      if (strlen(dev) < ARP_TABLE_ENTRY_LEN)
        strncpy(entry->ifname, dev, ARP_TABLE_ENTRY_LEN);
      else
        syslog(LOG_INFO, "Error during ARP table parsing");

      /* do not add routes for incomplete entries */
      if (debug && entry->want_route != !incomplete)
        printf("%s(%s): set want_route %d\n", inet_ntoa(entry->ipaddr_ia), entry->ifname,
               !incomplete);
      entry->want_route = !incomplete;

      std::cerr << "entry->want_route " << entry->want_route << "\n";

      /* Remove route from kernel if it already exists through
         a different interface */
      if (entry->want_route) {
        if (arpTable.remove_other_routes(entry->ipaddr_ia, entry->ifname) > 0)
          if (debug)
            printf("Found ARP entry %s(%s), removed entries via other "
                   "interfaces\n",
                   inet_ntoa(entry->ipaddr_ia), entry->ifname);
      }

      time(&entry->tstamp);

      if (debug && !entry->route_added && entry->want_route) {
        printf("arptab entry: '%s' HWAddr: '%s' Dev: '%s' route_added:%d "
               "want_route:%d\n",
               inet_ntoa(entry->ipaddr_ia), entry->hwaddr, entry->ifname, entry->route_added,
               entry->want_route);
      }
    }
  }

  if (fileSystem.fclose(arpf)) {
    errstr = strerror(errno);
    syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
  }
}
