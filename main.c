#include <sys/inotify.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include "queue.h"

//This path has to be set by user, depends of file system structure
#define BACKUP_DIR_PATH "/home/fayjulia/Desktop/backups"
#define MAX 100


// Function signatures
static void handle_events (int fd, int *wd, int argc, char* argv[], queue_t*
                           queue); 
void back_up (queue_t* queue, char* watched);
int isDirectoryEmpty();

char* create_backup_dir();
void create_soft_links(char* backup_folder_path, char* prev_backup_file);
void copy_files (int is_dir, char* source, char* destination);
void handle_queue(char* backup_folder_path, queue_t* queue, char* watched);

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
  
  //if dir does not already exist it will be created
  mkdir (BACKUP_DIR_PATH, 0777);

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
    back_up(queue, argv[1]); //
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
        //printf ("%d\n", queue->head);
        printf("IN_DELETE: ");
      }
      if (event->mask & IN_CREATE) {
        queue_put (queue, event->name, IN_CREATE);
        //printf ("%d\n", queue->head);
        printf("IN_CREATE: ");
      }
      if (event->mask & IN_MOVED_FROM) {
        queue_put (queue, event->name, IN_DELETE);
        //printf ("%d\n", queue->head);
        printf("IN_MOVED_FROM: ");
      }
      if (event->mask & IN_MOVED_TO) {
        queue_put (queue, event->name, IN_CREATE);
        //printf ("%d\n", queue->head);
        printf("IN_MOVED_TO: ");
      }
      if (event->mask & IN_MODIFY) {
        queue_put (queue, event->name, IN_MODIFY);
        //printf ("%d\n", queue->head);
        printf("IN_MODIFY: ");
      }

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

void back_up (queue_t* queue, char* watched) {
  /* first back up? create backup folder and folder for first back up */
  /* then copy over */
  /* create_new_dir if is not first back up */
  /* write soft links */
  /* then deal with queue, with different functions for different masks */
  
  int directory_status = isDirectoryEmpty();

  char prev_backup[MAX];
  
  //Finds most recent backup if backup folder is not empty
  if (directory_status == 0) {
     // The most recent backup folder created
    FILE *fp;
    char* command = calloc (sizeof (char), sizeof (char) * MAX);

    // Command for find last backup directory
    strcat(command, "cd ");
    strcat(command, BACKUP_DIR_PATH);
    strcat(command, "; ls | tail -1"); // Create ls command

    //printf("command = %s\n", command);

    // Call the ls command and read it from output.
   
    fp = popen(command, "r");
    if (fp == NULL) {
      printf("Failed to run command\n" );
      exit(1);
    }

    // Read the output of our ls command,
    // which is the most recent backup directory created.
    while (fgets(prev_backup, MAX, fp) != NULL) {
    }
    
    strcpy (strtok (prev_backup, "\n"), prev_backup);
    
    pclose(fp);
  }
  
  // Check if backup directory is empty.
  // If yes, then create_backup_dir and make a total copy of the folder
  // If not, then make soft links backwards to last backup folder with
  // each file represented
  char * backup_folder_path = create_backup_dir();

  // Thus directory has 1 element (was empty before we created another folder)
  // Copy all elements from the watched directory to the destination
  // (the backup folder) 
  if (directory_status == 1) {
    copy_files(1, watched, backup_folder_path);
    return;
  }
  
  // Otherwise, there is more than 1 backup folder

   /* If new file name is found in the old backup, make softlink between the 2 */
  create_soft_links(backup_folder_path, prev_backup);
   /* Handle all events that happened: modify, new, delete, rename   */
  handle_queue(backup_folder_path, queue, watched);
  
}


void create_soft_links(char* backup_folder_path, char* prev_backup_file) {
  //http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory
  //-\in-a-c-program
  DIR           *d;
  struct dirent *dir;
  
  char* prev_dir_path = calloc (sizeof (char), sizeof (char) * MAX);
  
  strcat (prev_dir_path, BACKUP_DIR_PATH);
  strcat (prev_dir_path, "/");
  strcat (prev_dir_path, prev_backup_file);
  strcat (prev_dir_path, "/");
  
  d = opendir(prev_dir_path);
  
  if (d == NULL) {
    perror ("opendir");
    exit (EXIT_FAILURE);
  }
  
  //iterate through files in directory and make softlinks back to previous
  //backup
  
  if (d)
    {
      int count = 0; // used to avoid reading "." and ".." directories
      while ((dir = readdir(d)) != NULL)
        {
          if (count >= 2) {
            char* new_file_in_backup =
              calloc (sizeof (char), sizeof (char) * MAX);
             char* old_file =
              calloc (sizeof (char), sizeof (char) * MAX);
            strcat (new_file_in_backup, backup_folder_path);
            strcat (new_file_in_backup, "/");
            strcat (new_file_in_backup, dir->d_name);
            strcat (old_file, "../");
            strcat (old_file, prev_backup_file);
            strcat (old_file, "/");
            strcat (old_file, dir->d_name);
             
            int success = symlink (old_file, new_file_in_backup);
            if (success == -1) {
              perror ("symlink");
              exit (EXIT_FAILURE);
            }
          }
          count++;
        }

      closedir(d);
    }
}

// To check if file exists from http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
void handle_queue(char* backup_folder_path, queue_t* queue, char* watched) {
  // Dequeue events until the queue is empty
  int counter = 0;
  while (queue != NULL) {
    counter++;
    printf ("counter %d\n", counter);
    /* printf ("%s\n", queue->head->filename); */
    /* printf ("mask1 %d\n", queue->head->mask); */
    /* printf ("%s\n", queue->head->next->filename); */
    /* printf ("mask2 %d\n", queue->head->next->mask); */
    /* printf ("%d\n", queue->head); */
    node_t* event = queue_take(queue);
    /* printf ("2%s\n", queue->head->filename); */

    if (event == NULL)
      return;
        
    char* copy_file_path = calloc(sizeof(char), sizeof(char) * MAX);
    strcat (copy_file_path, watched);
    strcat (copy_file_path, "/");
    strcat (copy_file_path, event->filename);

    //if the file that will be copied, modified or deleted still exists
    if (access( copy_file_path, F_OK )!=-1) {
      // If create event, copy the file from the source to the new backup dir
      if (event->mask & IN_CREATE) {
        printf ("in create\n");
        copy_files (0, copy_file_path, backup_folder_path); 
      }
      // If delete event, remove the softlink from the folder 
      else if (event->mask & IN_DELETE) {
        printf ("in delete\n");
        int status = unlink (copy_file_path);
        if (status == -1) {
          perror ("unlink");
          exit (EXIT_FAILURE);
        }
      }
      // If modify event
      // Copy over file IF the file does exist as a soft link
      else if (event->mask & IN_MODIFY) {
        printf ("in modify\n");
        int status = unlink (copy_file_path);
        if (status == -1) {
          perror ("unlink");
          exit (EXIT_FAILURE);
        }
        copy_files (0, copy_file_path, backup_folder_path);
      }
    }
  }
}


// Function to generate new directory with new name 

char* create_backup_dir(){
  // Malloc space for directory name of current backup date and time
  char* date_time_string = calloc (sizeof(char), sizeof (char) * 25 +
                                   sizeof (BACKUP_DIR_PATH));
  
  // Get time and local time
  time_t rawtime;   
  time ( &rawtime );
  struct tm* tm = localtime ( &rawtime );

  // Malloc space for string version of year, month, etc.
  char* intstr = malloc (sizeof (char) * 5);

  // Convert int fields of local time (year, month, etc.) to strings
  // Add string representations of date and time to new backup dir name
  strcat (date_time_string, BACKUP_DIR_PATH);
  strcat (date_time_string, "/");
  sprintf (intstr, "%04d", tm->tm_year + 1900);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "-");
  sprintf (intstr, "%02d", tm->tm_mon+1);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "-");
  sprintf (intstr, "%02d", tm->tm_mday);
  strcat (date_time_string, intstr);
  strcat (date_time_string, "--");
  sprintf (intstr, "%02d", tm->tm_hour);
  strcat (date_time_string, intstr);
  strcat (date_time_string, ":");
  sprintf (intstr, "%02d", tm->tm_min);
  strcat (date_time_string, intstr);
  strcat (date_time_string, ":");
  sprintf (intstr, "%02d", tm->tm_sec);
  strcat (date_time_string, intstr);

  int status = mkdir(date_time_string, 0700);
  if(status == -1) {
    perror("Failed to make a new backup directory");
    exit(EXIT_FAILURE);
  }
  return date_time_string;
}


void copy_files (int is_dir, char* source, char* destination) {
  
  char* command = calloc (sizeof (char), sizeof (char) * MAX);
  if (is_dir) {
    strcat (command, "cp -a ");
  }
  else {
    strcat (command, "cp ");
  }

  strcat (command, source);
  strcat (command, " ");
  strcat (command, destination);
  
  int success = system (command);
  if (success == -1) {
    perror ("system");
    exit (EXIT_FAILURE);
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
