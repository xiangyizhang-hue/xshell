#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char history[MAX_HISTORY][MAX_CMD_LENGTH];
static int history_count = 0;

void init_history() {
    history_count = 0;
}

void add_to_history(const char *command) {
    if (history_count < MAX_HISTORY) {
        strncpy(history[history_count], command, MAX_CMD_LENGTH - 1);
        history[history_count][MAX_CMD_LENGTH - 1] = '\0';
        history_count++;
    } else {
        // 如果历史记录已满，则丢弃最旧的一条，然后添加新的
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i+1]);
        }
        strncpy(history[MAX_HISTORY - 1], command, MAX_CMD_LENGTH - 1);
        history[MAX_HISTORY - 1][MAX_CMD_LENGTH - 1] = '\0';
    }
}

void xhistory() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i+1, history[i]);
    }
}
