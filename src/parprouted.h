/* parprouted: ProxyARP routing daemon.
 * (C) 2008 Vladimir Ivaschenko <vi@maks.net>
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

#define PROC_ARP "/proc/net/arp"
#define ARP_LINE_LEN 255
#define ARP_TABLE_ENTRY_LEN 20
#define ARP_TABLE_ENTRY_TIMEOUT 60 /* seconds */
#define ROUTE_CMD_LEN 255
#define SLEEPTIME 1000000 /* ms */
#define REFRESHTIME 50    /* seconds */
#define MAX_IFACES 10

#define MAX_RQ_SIZE 50 /* maximum size of request queue */

#define VERSION "0.7"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

struct arptab_entry {
  struct in_addr ipaddr_ia {};
  char hwaddr[ARP_TABLE_ENTRY_LEN] = "";
  char ifname[ARP_TABLE_ENTRY_LEN] = "";
  time_t tstamp{};
  int route_added{false};
  int incomplete{false};
  int want_route{false};
  struct arptab_entry *next = nullptr;
};

extern int debug;
extern int verbose;

extern int option_arpperm;

extern arptab_entry *arptab;
extern pthread_mutex_t arptab_mutex;
extern pthread_mutex_t req_queue_mutex;

arptab_entry *replace_entry(struct in_addr ipaddr, const char *dev);

extern const char *ifaces[MAX_IFACES];
extern int last_iface_idx;

struct FileSystem;
extern void *arp_thread(const char *ifname, FileSystem &);
extern void refresharp(arptab_entry *list);
extern void arp_req(const char *ifname, struct in_addr remaddr, int gratuitous);

extern void parseproc(FileSystem &);
extern void processarp(int cleanup);

extern void sighandler(int);
void *main_thread(FileSystem &fileSystem);
