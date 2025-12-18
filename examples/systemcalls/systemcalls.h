#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>    // Für exit() und EXIT_FAILURE
#include <unistd.h>    // Für fork(), execv(), dup2() und close()
#include <sys/wait.h>  // Für waitpid(), WIFEXITED und WEXITSTATUS
#include <fcntl.h>     // Für open() und die O_ Flags (O_WRONLY, etc.)

bool do_system(const char *command);

bool do_exec(int count, ...);

bool do_exec_redirect(const char *outputfile, int count, ...);
