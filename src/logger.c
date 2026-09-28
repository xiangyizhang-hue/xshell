#include "logger.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_FILE "xshell.log"

void init_logger() {
    // 可以在这里进行日志初始化，比如打开日志文件等
    // 目前我们只需要在每次写日志时打开文件，所以这里可以空着
}

void log_command(const char *command) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("fopen");
        return;
    }

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0'; // 去掉换行符

    fprintf(log_file, "[%s] COMMAND: %s\n", time_str, command);
    fclose(log_file);
}

void log_error(const char *error_msg) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("fopen");
        return;
    }

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';

    fprintf(log_file, "[%s] ERROR: %s\n", time_str, error_msg);
    fclose(log_file);
}

void xjournalctl() {
    FILE *log_file = fopen(LOG_FILE, "r");
    if (log_file == NULL) {
        perror("fopen");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), log_file) != NULL) {
        printf("%s", line);
    }

    fclose(log_file);
}
