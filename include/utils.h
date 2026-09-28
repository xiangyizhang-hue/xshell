#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 64
#define MAX_PATH_LENGTH 4096

// 命令解析相关函数
void parse_command(char *command, char **args, int *arg_count);
int is_builtin_command(char *command);
char* get_full_path(char *command);

// 字符串处理函数
void trim_string(char *str);
int starts_with(const char *str, const char *prefix);

#endif
