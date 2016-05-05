#include "backup_helpers.h"


#define BACKUP_DIR_PATH "/home/hardtmad/Desktop/backups"
#define MAX 100

// Function to check if the backup directory is empty
// (aside from standard . and ..)
// Citation: function taken from http://stackoverflow.com/questions/6383584/check-if-a-directory-is-empty-using-c-on-linux
// Returns 1 if empty indicating that we have not yet made any backups
int isDirectoryEmpty() {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(BACKUP_DIR_PATH);
  if (dir == NULL) { //Not a directory or doesn't exist
    perror("Backup directory does not exist.\n");
    exit (EXIT_FAILURE);
  }
  // Count how many files are in backup directory
  while ((d = readdir(dir)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) //Directory empty (contains only . and ..)
    return 1;
  else
    return 0;
}


// Function to generate new directory with new name based on date and time at
// directory creation
// Returns string of new directory absolute path
char* create_backup_dir(){
  // Malloc space for directory name of current backup date and time
  char* date_time_string = calloc (sizeof(char), sizeof (char) * MAX +
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

  // Make directory 
  int status = mkdir(date_time_string, 0700);
  if(status == -1) {
    perror("Failed to make a new backup directory");
    exit(EXIT_FAILURE);
  }
  return date_time_string;
}


// Funciton to copy file[s] from source to destination.
// If source is a directory, copy all files
void copy_files (int is_dir, char* source, char* destination) {
  // Use strcat to create  a string with the copy command we want call
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

  // Call command using system function which executes a shell command
  int success = system (command);
  if (success == -1) {
    perror ("system");
    exit (EXIT_FAILURE);
  }
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
