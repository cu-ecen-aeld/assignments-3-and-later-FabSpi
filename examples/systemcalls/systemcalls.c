#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int ret = system(cmd);

    if (ret == -1) {
        return false;
    }

    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    pid_t pid = fork(); // beide Prozesse (parent und child) existieren jz und arbeiten beide den Code ab

    if (pid == -1) {
        // Fork failed
        perror("fork");
        return false;
    } else if (pid == 0) { // if branch für den child process

        // Execute the command. command[0] is the path, command is the argument array.
        execv(command[0], command);
        
        // If execv returns, it failed
        perror("execv");
        exit(EXIT_FAILURE); 
    } else { // if branch für den parent process
        int status;
        // Wait for the specific child process to finish
        if (waitpid(pid, &status, 0) == -1) {   // waitpid pausiert hier und wartet auf den child process, waitpid wartet dabei auf den child process mit der spezifischen process id
            perror("waitpid");
            return false;
        }

        // 4. Check if the child exited successfully
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        } else {
            return false;
        }
    }

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int pid = fork();

    if (pid == -1) {
        perror("fork");
        return false;
    } 
    else if (pid == 0) {
        // KINDPROZESS
        
        // 1. Datei öffnen
        // O_WRONLY: Nur Schreiben
        // O_CREAT: Erstellen, falls nicht vorhanden
        // O_TRUNC: Inhalt löschen, falls vorhanden
        // 0644: Dateiberechtigungen (rw-r--r--)
        int fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // copy file descriptor of file to standard output (1) --> all outputs of the command will be written to the file istead of the console
        if (dup2(fd, 1) < 0) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // 3. Den alten Dateideskriptor schließen (wird nicht mehr gebraucht)
        close(fd);

        // execute program given by the command line which does not know that is writes into a file when writing to fd=1
        execv(command[0], command);
        
        // Falls execv fehlschlägt:
        perror("execv");
        exit(EXIT_FAILURE);
    } 
    else {
        // ELTERNPROZESS
        int status;
        if (waitpid(pid, &status, 0) == -1) {   //wait for child process
            return false;
        }
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }

    va_end(args);

    return true;
}
