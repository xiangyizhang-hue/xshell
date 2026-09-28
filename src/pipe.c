#include "pipe.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

int handle_pipes(char **args, int arg_count) {
    int i, pipe_count = 0;
    int pipe_positions[MAX_ARGUMENTS];

    // 查找管道符号
    for (i = 0; i < arg_count; i++) {
        if (args[i] != NULL && strcmp(args[i], "|") == 0) {
            pipe_positions[pipe_count++] = i;
        }
    }

    if (pipe_count == 0) {
        return 0; // 没有管道
    }

    // 处理多个管道
    int prev_pipe[2] = {-1, -1};
    int next_pipe[2];

    for (i = 0; i <= pipe_count; i++) {
        int start = (i == 0) ? 0 : pipe_positions[i-1] + 1;
        int end = (i == pipe_count) ? arg_count : pipe_positions[i];

        // 创建当前命令的参数数组
        char *cmd_args[MAX_ARGUMENTS];
        int cmd_arg_count = 0;

        for (int j = start; j < end; j++) {
            cmd_args[cmd_arg_count++] = args[j];
        }
        cmd_args[cmd_arg_count] = NULL;

        if (cmd_arg_count == 0) {
            fprintf(stderr, "syntax error: empty command in pipe\n");
            return -1;
        }

        // 如果不是最后一个命令，创建管道
        if (i < pipe_count) {
            if (pipe(next_pipe) == -1) {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) { // 子进程
            signal(SIGINT, SIG_DFL);
            // 设置输入重定向（从上一个管道读取）
            if (i > 0) {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            // 设置输出重定向（写入下一个管道）
            if (i < pipe_count) {
                dup2(next_pipe[1], STDOUT_FILENO);
                close(next_pipe[0]);
                close(next_pipe[1]);
            }

            // 执行命令
            if (execvp(cmd_args[0], cmd_args) == -1) {
                perror("execvp");
                _exit(127);
            }
        } else { // 父进程
            // 关闭不再需要的管道端
            if (i > 0) {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            if (i < pipe_count) {
                prev_pipe[0] = next_pipe[0];
                prev_pipe[1] = next_pipe[1];
            }
        }
    }

    // 等待所有子进程完成
    for (i = 0; i <= pipe_count; i++) {
        wait(NULL);
    }

    return 1;
}
