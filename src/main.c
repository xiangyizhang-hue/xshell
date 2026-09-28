#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include "utils.h"
#include "builtin.h"
#include "external.h"
#include "redirect.h"
#include "pipe.h"
#include "history.h"
#include "logger.h"

void display_prompt() {
    char cwd[MAX_PATH_LENGTH];
    char hostname[256];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "?");
    }

    // 简化提示符，避免使用 gethostname
    char *username = getenv("USER");
    if (username == NULL) {
        username = "user";
    }

    printf("[%s@localhost %s]# ", username, cwd);
    fflush(stdout);
}

void handle_signal(int sig) {
    (void)sig;
    const char newline = '\n';
    write(STDOUT_FILENO, &newline, 1);
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGUMENTS];
    int arg_count;

    // 初始化
    init_history();
    init_logger();
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    printf("Welcome to XShell - A simple bash-like shell\n");
    printf("Type 'quit' to exit\n\n");

    while (1) {
        display_prompt();

        // 读取命令
        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nExiting XShell...\n");
                break;
            }
            clearerr(stdin);
            continue;
        }

        // 去除换行符
        command[strcspn(command, "\n")] = '\0';
        trim_string(command);

        // 跳过空命令
        if (strlen(command) == 0) {
            continue;
        }

        // 记录命令到历史
        add_to_history(command);
        log_command(command);

        // 解析命令
        parse_command(command, args, &arg_count);

        if (arg_count == 0) {
            continue;
        }

        // 检查是否有管道
        int has_pipe = 0;
        for (int i = 0; i < arg_count; i++) {
            if (args[i] != NULL && strcmp(args[i], "|") == 0) {
                has_pipe = 1;
                break;
            }
        }

        if (has_pipe) {
            // 处理管道命令
            if (handle_pipes(args, arg_count) == -1) {
                log_error("Pipe command execution failed");
            }
            continue;
        }

        // 检查重定向
        int redirect_result = handle_redirection(args, &arg_count);
        if (redirect_result == -1) {
            log_error("Redirection setup failed");
            continue;
        }

        // 执行命令
        int executed = 0;

        if (arg_count > 0) {
            // 尝试执行内置命令
            executed = execute_builtin(args, arg_count);

            if (!executed) {
                // 尝试执行外部命令
                executed = execute_external(args, arg_count);

                if (!executed) {
                    fprintf(stderr, "xshell: command not found: %s\n", args[0]);
                    log_error("Command not found");
                }
            }
        }

        // 恢复重定向
        if (redirect_result > 0) {
            restore_stdout();
            restore_stderr();
        }
    }

    return 0;
}
