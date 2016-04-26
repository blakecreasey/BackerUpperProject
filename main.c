#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>

#define SYS_PATH ./Documents/test_directory

// Function signatures
static void handle_events (int fd, int *wd, int argc, char* argv[]); 
void add_file (char* filename);
void delete_file (char* filename);
void change_filename (char* filename, char* new_filename); 

// Main function
// Basis taken from Linux man page for inotify
int main(int argc, char* argv[]) {
  char buf;
  int fd, i, poll_num;
  int *wd; // Watch descriptor for inotify event
  nfds_t nfds; // Unsigned int used for file descriptoqrs
  struct pollfd fds[2]; /* Set of file descriptors to be monitored
                           Fields: int fd -- file descriptor
                                   short events -- requested events
                                   short revents -- returned events */       

  // Take in files info from command line
  if (argc < 2) {
    printf("Usage: %s PATH [PATH ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Press ENTER key to terminate.\n");

  /* Create the file descriptor for accessing the inotify API */
  // Initialize a new inotify instance
  // Return fd, a file descriptor associated with our inotify event queue
  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }

  /* Allocate memory for argc watch descriptors and set all entries to 0 */
  wd = calloc(argc, sizeof(int));
  if (wd == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  /* Mark directories for events
     - file was opened
     - file was closed */

  for (i = 1; i < argc; i++) {
    // Declare array of watch descriptors which hold file descriptors of inotify
    // events
    wd[i] = inotify_add_watch(fd, argv[i],
                              IN_OPEN | IN_CLOSE);
    if (wd[i] == -1) {
      fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
      perror("inotify_add_watch");
      exit(EXIT_FAILURE);
    }
  }

  /* Prepare for polling */

  nfds = 2;

  /* Console input */

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  /* Inotify input */

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  /* Wait for events and/or terminal input */

  printf("Listening for events.\n");
  while (1) {
    poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {

      if (fds[0].revents & POLLIN) {

        /* Console input is available. Empty stdin and quit */

        while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
          continue;
        break;
      }

      if (fds[1].revents & POLLIN) {

        /* Inotify events are available */

        handle_events(fd, wd, argc, argv);
      }
    }
  }

  printf("Listening for events stopped.\n");

  /* Close inotify file descriptor */

  close(fd);

  free(wd);
  exit(EXIT_SUCCESS);
}

// Method taken from Linux man page
static void handle_events (int fd, int *wd, int argc, char* argv[]) {
  /* Some systems cannot read integer variables if they are not
     properly aligned. On other systems, incorrect alignment may
     decrease performance. Hence, the buffer used for reading from
     the inotify file descriptor should have the same alignment as
     struct inotify_event. */

  char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  int i;
  ssize_t len;
  char *ptr;

  /* Loop while events can be read from inotify file descriptor. */

  for (;;) {

    /* Read some events. */

    len = read(fd, buf, sizeof buf);
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /* If the nonblocking read() found no events to read, then
       it returns -1 with errno set to EAGAIN. In that case,
       we exit the loop. */

    if (len <= 0)
      break;

    /* Loop over all events in the buffer */

    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {

      event = (const struct inotify_event *) ptr;

      /* Print event type */

      if (event->mask & IN_OPEN)
        printf("IN_OPEN: ");
      if (event->mask & IN_CLOSE_NOWRITE)
        printf("IN_CLOSE_NOWRITE: ");
      if (event->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE: ");

      /* Print the name of the watched directory */

      for (i = 1; i < argc; ++i) {
        if (wd[i] == event->wd) {
          printf("%s/", argv[i]);
          break;
        }
      }

      /* Print the name of the file */

      if (event->len)
        printf("%s", event->name);

      /* Print type of filesystem object */

      if (event->mask & IN_ISDIR)
        printf(" [directory]\n");
      else
        printf(" [file]\n");
    }
  }
}

void add_file (char* filename) {

}

void delete_file (char* filename) {

}

void change_filename (char* filename, char* new_filename){

}



/*
Works Cited: 
http://man7.org/linux/man-pages/man7/inotify.7.html
 */
