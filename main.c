#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <string.h>


#define BACKUP_DIR_PATH "~/Desktop/backups"

// Define a struct for queue nodes to track events
  typedef struct node {
    uint32_t mask;
    const char* filename;
    struct node* next;
  } node_t;
  
  typedef struct queue {
    node_t* head;
    node_t* tail;
  } queue_t;
  

// Function signatures
static void handle_events (int fd, int *wd, int argc, char* argv[], queue_t*
                           queue); 
void add_file (char* filename);
void delete_file (char* filename);
void change_filename (char* filename, char* new_filename);
void back_up (queue_t* queue, char* watched);
int isDirectoryEmpty();

queue_t* queue_create();
void queue_put(queue_t* queue, const char* filename, uint32_t mask);
node_t* queue_take(queue_t* queue);

// Main function
// Basis taken from Linux man page for inotify
int main(int argc, char* argv[]) {

  //Initialize variables
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
                              IN_MODIFY | IN_MOVED_TO |
                              IN_DELETE | IN_CREATE | IN_MOVED_FROM);
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

  queue_t* queue = queue_create();

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
        handle_events(fd, wd, argc, argv, queue);
      }
    }
  } // while

  int back_up_notice = 0;
  printf("Do you want to backup?, 0 no, 1 yes\n");
  scanf("%d", &back_up_notice);
  if (back_up_notice == 1) {
    printf("it worked!\n");
    back_up(queue, argv[1]);
  }
  printf("Listening for events stopped.\n");

  /* Close inotify file descriptor */

  close(fd);

  free(wd);
  exit(EXIT_SUCCESS);
}

// Method taken from Linux man page
static void handle_events (int fd, int *wd, int argc, char* argv[],
                           queue_t* queue) {
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
      if (event->mask & IN_DELETE) {
        queue_put (queue, event->name, IN_DELETE);
        printf("IN_DELETE: ");
      }
      if (event->mask & IN_CREATE) {
        queue_put (queue, event->name, IN_CREATE);
        printf("IN_CREATE: ");
      }
      /* if (event->mask & IN_DELETE_SELF) */
      /*   printf("IN_DELETE_SELF: "); */
      /* if (event->mask & IN_MOVE_SELF) */
      /*   printf("IN_MOVE_SELF: "); */
      if (event->mask & IN_MOVED_FROM) {
        queue_put (queue, event->name, IN_DELETE);
        printf("IN_MOVED_FROM: ");
      }
      if (event->mask & IN_MOVED_TO) {
        queue_put (queue, event->name, IN_CREATE);
        printf("IN_MOVED_TO: ");
      }
      if (event->mask & IN_MODIFY) {
        queue_put (queue, event->name, IN_MODIFY);
        printf("IN_MODIFY: ");
      }
      /* if (event->mask & IN_OPEN) */
      /*   printf("IN_OPEN: "); */
      /* if (event->mask & IN_CLOSE_WRITE) */
      /*   printf("IN_CLOSE_WRITE: "); */
      /* if (event->mask & IN_CLOSE_NOWRITE) */
      /*   printf("IN_CLOSE_NOWRITE: "); */

      printf ("%d/n", (queue_take(queue))->mask);


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






/* QUEUE FUNCTIONS */
// Create a new empty queue
queue_t* queue_create() {
  queue_t* q = (queue_t*) malloc(sizeof(queue_t));
  if(q == NULL)
      perror("Malloc failed\n");
  q->head = NULL;
  //q->head->next = NULL;
  q->tail = NULL;
  return q;
}

// Put an element at the end of a queue
void queue_put(queue_t* queue, const char* filename, uint32_t mask) {
  node_t* newNode = (node_t*) malloc(sizeof(node_t));
  newNode->next = NULL;
  newNode->filename = filename;
  newNode->mask = mask;

  //check if queue is empty
  if(queue->head == NULL && queue->tail == NULL) {
    queue->head = queue->tail = newNode;
    return;
  }  
  queue->tail->next = newNode;
  queue->tail = newNode;
}

// Take an element off the front of a queue
node_t* queue_take(queue_t* queue) {
  node_t* node = queue->head;
  if (queue->head == NULL) {
    return NULL;
  }  
  else if (queue->head == queue->tail) {
    queue->head = queue->tail = NULL;
    return node;
  }
  else {
    queue->head = queue->head->next;
    return node;
  }
}

void back_up (queue_t* queue, char* watched) {
  /* first back up? create backup folder and folder for first back up */
  /* then copy over */
  /* create_new_dir if is not first back up */
  /* write soft links */
  /* then deal with queue, with different functions for different masks */

  // Get time and local time
  time_t rawtime;   
  time ( &rawtime );
  struct tm* tm = localtime ( &rawtime );

  // Malloc space for string version of year, month, etc.
  char* intstr = malloc (sizeof (char) * 5);

  // Malloc space for directory name of current backup date and time
  char* date_time_string = calloc (sizeof(char), sizeof (char) * 25 +
                                   sizeof (BACKUP_DIR_PATH));
  // Convert int fields of local time (year, month, etc.) to strings
  // Add string representations of date and time to new backup dir name
  strcat (date_time_string, BACKUP_DIR_PATH);
  strcat (date_time_string, "/");
  sprintf (intstr, "%d", tm->tm_year + 1900);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "-");
  sprintf (intstr, "%d", tm->tm_mon+1);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "-");
  sprintf (intstr, "%d", tm->tm_mday);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "--");
  sprintf (intstr, "%d", tm->tm_hour);
  strcat (date_time_string, intstr);
  strcat (date_time_string, ":");
  sprintf (intstr, "%d", tm->tm_min);
  strcat (date_time_string, intstr);
  strcat (date_time_string, ":");
  sprintf (intstr, "%d", tm->tm_sec);
  strcat (date_time_string, intstr);

  printf("new dir name: %s\n", date_time_string);
  

  // Check if our backup directory is empty indicating that this is our
  // first backup
  if (isDirectoryEmpty) {
    int status= mkdir(date_time_string
                      , 0700);
    //if (status == -1){
    //perror("mkdir");
    //exit(EXIT_FAILURE);
    //}    
  }
}

//taken from http://stackoverflow.com/questions/6383584/check-if-a-directory-is-empty-using-c-on-linux
int isDirectoryEmpty() {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(BACKUP_DIR_PATH);
  if (dir == NULL) //Not a directory or doesn't exist
    return 1;
  while ((d = readdir(dir)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) //Directory Empty
    return 1;
  else
    return 0;
}


/*
Works Cited: 
http://man7.org/linux/man-pages/man7/inotify.7.html
queue structs and functions taken from CSC-213 data structures lab also 
 by Zoe Wolter
 */
