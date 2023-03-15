#include "parprouted.h"

#include "context.h"
#include "fs.h"

#include <string>
#include <thread>

std::string progname{};

int last_thread_idx = -1;
std::thread my_threads[MAX_IFACES + 1];

int main(int argc, char **argv) {
  pid_t child_pid;
  int i;
  bool help = true;

  progname = basename(argv[0]);

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-d")) {
      debug = true;
      help = false;
    } else if (!strcmp(argv[i], "-p")) {
      option_arpperm = true;
      help = false;
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      break;
    } else {
      last_iface_idx++;
      ifaces[last_iface_idx] = argv[i];
      help = false;
    }
  }

  if (help || last_iface_idx <= -1) {
    printf("parprouted: proxy ARP routing daemon, version %s.\n", VERSION);
    printf("(C) 2007 Vladimir Ivaschenko <vi@maks.net>, GPL2 license.\n");
    printf("Usage: parprouted [-d] [-p] interface [interface]\n");
    exit(1);
  }

  if (!debug) {
    /* fork to go into the background */
    if ((child_pid = fork()) < 0) {
      fprintf(stderr, "could not fork(): %s", strerror(errno));
      exit(1);
    } else if (child_pid > 0) {
      /* fork was ok, wait for child to exit */
      if (waitpid(child_pid, NULL, 0) != child_pid) {
        perror(progname.c_str());
        exit(1);
      }
      /* and exit myself */
      exit(0);
    }
    /* and fork again to make sure we inherit all rights from init */
    if ((child_pid = fork()) < 0) {
      perror(progname.c_str());
      exit(1);
    } else if (child_pid > 0) {
      exit(0);
    }

    /* create our own session */
    setsid();

    /* close stdin/stdout/stderr */
    close(0);
    close(1);
    close(2);
  }

  openlog(progname.c_str(), LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
  syslog(LOG_INFO, "Starting.");

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);

  pthread_mutex_init(&arptab_mutex, NULL);
  pthread_mutex_init(&req_queue_mutex, NULL);

  auto fileSystem = makeFileSystem();
  auto context = makeContext();

  my_threads[++last_thread_idx] =
      std::thread(main_thread, std::ref(*fileSystem), std::ref(*context));

  for (i = 0; i <= last_iface_idx; i++) {
    my_threads[++last_thread_idx] =
        std::thread(arp_thread, ifaces[i], std::ref(*fileSystem), std::ref(*context));
    if (debug) {
      printf("Created ARP thread for %s.\n", ifaces[i]);
    }
  }

  my_threads[0].join();

  while (waitpid(-1, NULL, WNOHANG)) {
  }
  exit(1);
}
