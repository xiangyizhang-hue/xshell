#include "utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

void parse_command(char *command, char **args, int *arg_count) {
    int count = 0;
    char *token = strtok(command, " \t\n");

    while (token != NULL && count < MAX_ARGUMENTS - 1) {
        args[count] = token;
        count++;
        token = strtok(NULL, " \t\n");
    }
    args[count] = NULL;
    *arg_count = count;
}

int is_builtin_command(char *command) {
    if (command == NULL) return 0;

    char *builtins[] = {
        "xpwd", "xcd", "xls", "xtouch", "xecho", "xcat",
        "xcp", "xrm", "xmv", "xhistory", "xtee", "xjournalctl", "quit"
    };
    int num_builtins = sizeof(builtins) / sizeof(char*);

    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(command, builtins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

char* get_full_path(char *command) {
    if (command == NULL) return NULL;

    // 如果命令包含斜杠，则直接返回（认为是路径）
    if (strchr(command, '/') != NULL) {
        if (access(command, X_OK) == 0) {
            return strdup(command);
        }
        return NULL;
    }

    char *path = getenv("PATH");
    if (path == NULL) return NULL;

    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");

    while (dir != NULL) {
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }

        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

void trim_string(char *str) {
    if (str == NULL) return;

    int start = 0;
    int end = strlen(str) - 1;

    while (isspace(str[start])) start++;
    while (end >= start && isspace(str[end])) end--;

    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\0';
}

int starts_with(const char *str, const char *prefix) {
    if (str == NULL || prefix == NULL) return 0;
    size_t len_prefix = strlen(prefix);
    return strncmp(str, prefix, len_prefix) == 0;
}
