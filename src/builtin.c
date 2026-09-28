#include "builtin.h"
#include "utils.h"
#include "history.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

static int copy_path(const char *src, const char *dest);
static int remove_path_recursive(const char *path);

int execute_builtin(char **args, int arg_count) {
    if (arg_count == 0) return 0;

    char *command = args[0];

    if (strcmp(command, "xpwd") == 0) {
        xpwd();
        return 1;
    } else if (strcmp(command, "xcd") == 0) {
        xcd(args, arg_count);
        return 1;
    } else if (strcmp(command, "xls") == 0) {
        xls(args, arg_count);
        return 1;
    } else if (strcmp(command, "xtouch") == 0) {
        xtouch(args, arg_count);
        return 1;
    } else if (strcmp(command, "xecho") == 0) {
        xecho(args, arg_count);
        return 1;
    } else if (strcmp(command, "xcat") == 0) {
        xcat(args, arg_count);
        return 1;
    } else if (strcmp(command, "xcp") == 0) {
        xcp(args, arg_count);
        return 1;
    } else if (strcmp(command, "xrm") == 0) {
        xrm(args, arg_count);
        return 1;
    } else if (strcmp(command, "xmv") == 0) {
        xmv(args, arg_count);
        return 1;
    } else if (strcmp(command, "xhistory") == 0) {
        xhistory();
        return 1;
    } else if (strcmp(command, "xtee") == 0) {
        xtee(args, arg_count);
        return 1;
    } else if (strcmp(command, "xjournalctl") == 0) {
        xjournalctl();
        return 1;
    } else if (strcmp(command, "quit") == 0) {
        exit(0);
    }

    return 0;
}

void xpwd() {
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

void xcd(char **args, int arg_count) {
    char *path = NULL;

    if (arg_count == 1) {
        // 无参数，返回主目录
        path = getenv("HOME");
        if (path == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw != NULL) {
                path = pw->pw_dir;
            }
        }
    } else if (arg_count == 2) {
        if (strcmp(args[1], "-") == 0) {
            // 返回上一目录
            path = getenv("OLDPWD");
            if (path == NULL) {
                fprintf(stderr, "xcd: OLDPWD not set\n");
                return;
            }
        } else {
            path = args[1];
        }
    } else {
        fprintf(stderr, "xcd: too many arguments\n");
        return;
    }

    if (path != NULL) {
        char oldpwd[MAX_PATH_LENGTH];
        if (getcwd(oldpwd, sizeof(oldpwd)) == NULL) {
            perror("getcwd");
            return;
        }

        if (chdir(path) == 0) {
            setenv("OLDPWD", oldpwd, 1);
            char newpwd[MAX_PATH_LENGTH];
            if (getcwd(newpwd, sizeof(newpwd)) != NULL) {
                setenv("PWD", newpwd, 1);
            }
        } else {
            perror("chdir");
        }
    }
}

void xls(char **args, int arg_count) {
    char *path = ".";
    if (arg_count > 1) {
        path = args[1];
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue; // 跳过隐藏文件
        }

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                printf("Dir: %s/\n", entry->d_name);
            } else {
                printf("File: %s\n", entry->d_name);
            }
        } else {
            perror("stat");
        }
    }

    closedir(dir);
}

void xtouch(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "xtouch: missing file operand\n");
        return;
    }

    for (int i = 1; i < arg_count; i++) {
        if (access(args[i], F_OK) != 0) {
            // 文件不存在，创建新文件
            int fd = open(args[i], O_CREAT, 0644);
            if (fd == -1) {
                perror("open");
            } else {
                close(fd);
            }
        }
        // 如果文件已存在，什么都不做
    }
}

void xecho(char **args, int arg_count) {
    for (int i = 1; i < arg_count; i++) {
        printf("%s", args[i]);
        if (i < arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

void xcat(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "xcat: missing file operand\n");
        return;
    }

    for (int i = 1; i < arg_count; i++) {
        FILE *file = fopen(args[i], "r");
        if (file == NULL) {
            perror("fopen");
            continue;
        }

        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }

        fclose(file);
    }
}

void xcp(char **args, int arg_count) {
    if (arg_count < 3) {
        fprintf(stderr, "xcp: missing file operands\n");
        return;
    }

    // 检查是否递归复制目录
    int recursive = 0;
    int src_start = 1;

    if (strcmp(args[1], "-r") == 0) {
        recursive = 1;
        src_start = 2;

        if (arg_count < 4) {
            fprintf(stderr, "xcp: missing file operands\n");
            return;
        }
    }

    char *src = args[src_start];
    char *dest = args[src_start + 1];

    struct stat st;
    if (stat(src, &st) == -1) {
        perror("stat");
        return;
    }

    if (S_ISDIR(st.st_mode) && !recursive) {
        fprintf(stderr, "xcp: -r not specified; omitting directory '%s'\n", src);
        return;
    }

    if (copy_path(src, dest) != 0) perror("xcp");
}

void xrm(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "xrm: missing operand\n");
        return;
    }

    int recursive = 0;
    int target_start = 1;

    if (strcmp(args[1], "-r") == 0) {
        recursive = 1;
        target_start = 2;

        if (arg_count < 3) {
            fprintf(stderr, "xrm: missing operand\n");
            return;
        }
    }

    for (int i = target_start; i < arg_count; i++) {
        struct stat st;
        if (stat(args[i], &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            if (unlink(args[i]) == -1) {
                perror("unlink");
            }
        } else if (S_ISDIR(st.st_mode)) {
            if (recursive) {
                if (remove_path_recursive(args[i]) == -1) perror("xrm");
            } else {
                fprintf(stderr, "xrm: cannot remove '%s': Is a directory\n", args[i]);
            }
        }
    }
}

static int copy_file(const char *src, const char *dest, mode_t mode) {
    int input = open(src, O_RDONLY);
    if (input < 0) return -1;
    int output = open(dest, O_WRONLY | O_CREAT | O_TRUNC, mode & 0777);
    if (output < 0) { close(input); return -1; }
    char buffer[8192];
    ssize_t count;
    while ((count = read(input, buffer, sizeof(buffer))) > 0) {
        ssize_t written = 0;
        while (written < count) {
            ssize_t result = write(output, buffer + written, (size_t)(count - written));
            if (result < 0) { close(input); close(output); return -1; }
            written += result;
        }
    }
    int saved_errno = errno;
    close(input);
    close(output);
    errno = saved_errno;
    return count < 0 ? -1 : 0;
}

static int copy_path(const char *src, const char *dest) {
    struct stat st;
    if (lstat(src, &st) != 0) return -1;
    if (S_ISREG(st.st_mode)) return copy_file(src, dest, st.st_mode);
    if (!S_ISDIR(st.st_mode)) { errno = ENOTSUP; return -1; }
    if (mkdir(dest, st.st_mode & 0777) != 0 && errno != EEXIST) return -1;
    DIR *dir = opendir(src);
    if (!dir) return -1;
    struct dirent *entry;
    int result = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char src_child[MAX_PATH_LENGTH];
        char dest_child[MAX_PATH_LENGTH];
        if (snprintf(src_child, sizeof(src_child), "%s/%s", src, entry->d_name) >= (int)sizeof(src_child)
                || snprintf(dest_child, sizeof(dest_child), "%s/%s", dest, entry->d_name) >= (int)sizeof(dest_child)
                || copy_path(src_child, dest_child) != 0) {
            result = -1;
            break;
        }
    }
    closedir(dir);
    return result;
}

static int remove_path_recursive(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return -1;
    struct dirent *entry;
    int result = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char child[MAX_PATH_LENGTH];
        if (snprintf(child, sizeof(child), "%s/%s", path, entry->d_name) >= (int)sizeof(child)) {
            errno = ENAMETOOLONG;
            result = -1;
            break;
        }
        struct stat st;
        if (lstat(child, &st) != 0) { result = -1; break; }
        if (S_ISDIR(st.st_mode) ? remove_path_recursive(child) != 0 : unlink(child) != 0) {
            result = -1;
            break;
        }
    }
    closedir(dir);
    return result == 0 ? rmdir(path) : -1;
}

void xmv(char **args, int arg_count) {
    if (arg_count < 3) {
        fprintf(stderr, "xmv: missing file operands\n");
        return;
    }

    // 简化实现，只处理文件移动
    char *src = args[1];
    char *dest = args[2];

    if (rename(src, dest) == -1) {
        perror("rename");
    }
}

void xtee(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "xtee: missing file operand\n");
        return;
    }

    FILE *file = fopen(args[1], "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        printf("%s", buffer);
        fprintf(file, "%s", buffer);
    }

    fclose(file);
}
