#include "external.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

int execute_external(char **args, int arg_count) {
    if (arg_count == 0) return 0;

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // 子进程
        signal(SIGINT, SIG_DFL);
        execvp(args[0], args);
        perror("execvp");
        _exit(127);
    } else {
        // 父进程
        int status;
        waitpid(pid, &status, 0);
        return 1;
    }
}
