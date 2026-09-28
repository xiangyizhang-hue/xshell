#include "redirect.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static int stdout_backup = -1;
static int stderr_backup = -1;
static int redirect_stdout = 0;
static int redirect_stderr = 0;

int handle_redirection(char **args, int *arg_count) {
    int i, j;
    int redirect_found = 0;
    char *filename = NULL;
    int flags = O_WRONLY | O_CREAT;
    int mode = 0644;
    int redirect_type = 0; // 0: stdout, 1: stderr, 2: append stdout

    fflush(stdout);
    fflush(stderr);
    // 备份原始文件描述符
    if (stdout_backup == -1) {
        stdout_backup = dup(STDOUT_FILENO);
        stderr_backup = dup(STDERR_FILENO);
    }

    // 查找重定向符号
    for (i = 0; i < *arg_count; i++) {
        if (args[i] == NULL) continue;

        if (strcmp(args[i], ">") == 0) {
            redirect_type = 0;
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            redirect_found = 1;
        } else if (strcmp(args[i], ">>") == 0) {
            redirect_type = 0;
            flags = O_WRONLY | O_CREAT | O_APPEND;
            redirect_found = 2;
        } else if (strcmp(args[i], "2>") == 0) {
            redirect_type = 1;
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            redirect_found = 1;
        }

        if (redirect_found) {
            if (i + 1 >= *arg_count || args[i + 1] == NULL) {
                fprintf(stderr, "syntax error: no file specified for redirection\n");
                return -1;
            }

            filename = args[i + 1];

            // 打开文件
            int fd = open(filename, flags, mode);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            // 重定向
            if (redirect_type == 0) { // stdout
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    return -1;
                }
                redirect_stdout = redirect_found;
            } else { // stderr
                if (dup2(fd, STDERR_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    return -1;
                }
                redirect_stderr = 1;
            }

            close(fd);

            // 从参数列表中移除重定向符号和文件名
            args[i] = NULL;
            args[i + 1] = NULL;

            // 重新整理参数列表
            j = 0;
            for (int k = 0; k < *arg_count; k++) {
                if (args[k] != NULL) {
                    args[j++] = args[k];
                }
            }
            *arg_count = j;
            args[j] = NULL;

            break;
        }
    }

    return redirect_found;
}

void restore_stdout() {
    if (redirect_stdout) {
        fflush(stdout);
        if (dup2(stdout_backup, STDOUT_FILENO) == -1) {
            perror("dup2");
        }
        redirect_stdout = 0;
    }
}

void restore_stderr() {
    if (redirect_stderr) {
        fflush(stderr);
        if (dup2(stderr_backup, STDERR_FILENO) == -1) {
            perror("dup2");
        }
        redirect_stderr = 0;
    }
}
