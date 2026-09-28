#ifndef HISTORY_H
#define HISTORY_H

// 移除对 utils.h 的依赖，直接定义常量
#define MAX_HISTORY 1000
#define MAX_CMD_LENGTH 1024

void init_history(void);
void add_to_history(const char *command);
void xhistory(void);

#endif
