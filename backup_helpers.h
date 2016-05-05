#ifndef BACKUP_HELPERS_H
#define BACKUP_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int isDirectoryEmpty();
char* create_backup_dir();
void copy_files (int is_dir, char* source, char* destination);
void create_soft_links(char* backup_folder_path, char* prev_backup_file);


#endif
