#README file

###General Instructions:

Preconditions:
Our system will not iterate through directories within a watched
directory. Any file which the user wishes to backup must be directly in the
watched directory.
Don't end the path for the watched directory with a slash, for example,
write '.' for current directory not './'

To use our system the user has to specify where they want their backup
directory in the main file and backup helper file.The user then must run
the program with a
command line argument that specifies the directory to be watched, e.g.
./main ~/Desktop/CSC213

The user may then edit documents in that directory and our program will
monitor and log the changes until the user presses enter to exit the
program. The program will then prompt the user to either backup their
changes, 1, or to not backup in the backup direcory, 0. If the user decides
to backup the changes, if these are the first changes, all files from the
watched directory will be copied over to a time-stamped directory within
the backup directory, otherwise a time-stamped directory will be created
and any changed files will be copied over (all others will be soft
linked to the most recent backup version).

### Specific Example
Here is example output, for when the current directory is watched and the
README.md file is modified.

zermelo$ ./main .
Press ENTER key to terminate.
Listening for events.
IN_CREATE: ./.#README.md [file]
IN_MODIFY: ./README.md [file]
IN_DELETE: ./.#README.md [file]

Do you want to backup?, 0 no, 1 yes
1
Listening for events stopped.

The new directory created in our backup directory:
/home/fayjulia/Desktop/backups/2016-05-08--13:30:03/
Files that have not been changed are softlinked to their most recent
backups in the last backup directory:
/home/fayjulia/Desktop/backups/2016-05-08--13:29:23/
We expect there to be a new copy of README with the changes made in the new
dir in the backup dir. In the new directory there will be soft links to the
most recent backups of the other files we are watching.






