#include "parprouted.h"

#include "arp-table.h"
#include "fs.h"

char *progname;

int last_thread_idx = -1;
pthread_t my_threads[MAX_IFACES + 1];

static int perform_shutdown = 0;

void cleanup(void *) {
  /* FIXME: I think this is a wrong way to do it ... */

  syslog(LOG_INFO, "Received signal; cleaning up.");
  /*
  for (i=0; i <= last_thread_idx; i++) {
      pthread_cancel(my_threads[i]);
  }
  */
  pthread_mutex_trylock(&arptab_mutex);
  processarp(1);
  syslog(LOG_INFO, "Terminating.");
  exit(1);
}

void sighandler(int) {
  /* FIXME: I think this is a wrong way to do it ... */
  perform_shutdown = 1;
}

void *main_thread(void *) {
  auto arpTable = makeArpTable();
  auto fileSystem = makeFileSystem();

  time_t last_refresh;

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_cleanup_push(cleanup, NULL);
  while (1) {
    if (perform_shutdown) {
      pthread_exit(0);
    }
    pthread_testcancel();
    pthread_mutex_lock(&arptab_mutex);
    parseproc(*arpTable, *fileSystem);
    processarp(0);
    pthread_mutex_unlock(&arptab_mutex);
    usleep(SLEEPTIME);
    if (!option_arpperm && time(NULL) - last_refresh > REFRESHTIME) {
      pthread_mutex_lock(&arptab_mutex);
      refresharp(*arpTable);
      pthread_mutex_unlock(&arptab_mutex);
      time(&last_refresh);
    }
  }
  /* required since pthread_cleanup_* are implemented as macros */
  pthread_cleanup_pop(0);
}

int main(int argc, char **argv) {
  pid_t child_pid;
  int i, help = 1;

  progname = (char *)basename(argv[0]);

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-d")) {
      debug = 1;
      help = 0;
    } else if (!strcmp(argv[i], "-p")) {
      option_arpperm = 1;
      help = 0;
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      break;
    } else {
      last_iface_idx++;
      ifaces[last_iface_idx] = argv[i];
      help = 0;
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
        perror(progname);
        exit(1);
      }
      /* and exit myself */
      exit(0);
    }
    /* and fork again to make sure we inherit all rights from init */
    if ((child_pid = fork()) < 0) {
      perror(progname);
      exit(1);
    } else if (child_pid > 0)
      exit(0);

    /* create our own session */
    setsid();

    /* close stdin/stdout/stderr */
    close(0);
    close(1);
    close(2);
  }

  openlog(progname, LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
  syslog(LOG_INFO, "Starting.");

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);

  pthread_mutex_init(&arptab_mutex, NULL);
  pthread_mutex_init(&req_queue_mutex, NULL);

  if (pthread_create(&my_threads[++last_thread_idx], NULL, main_thread, NULL)) {
    syslog(LOG_ERR, "Error creating main thread.");
    abort();
  }

  for (i = 0; i <= last_iface_idx; i++) {
    if (pthread_create(&my_threads[++last_thread_idx], NULL,
                       reinterpret_cast<void *(*)(void *)>(arp), (void *)ifaces[i])) {
      syslog(LOG_ERR, "Error creating ARP thread for %s.", ifaces[i]);
      abort();
    }
    if (debug)
      printf("Created ARP thread for %s.\n", ifaces[i]);
  }

  if (pthread_join(my_threads[0], NULL)) {
    syslog(LOG_ERR, "Error joining thread.");
    abort();
  }

  while (waitpid(-1, NULL, WNOHANG)) {
  }
  exit(1);
}
