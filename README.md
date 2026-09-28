# xShell 自定义 Linux 命令解释器

xShell 是一个用于学习 Linux 进程与文件描述符的教学型 Shell。项目按 `main`、`builtin`、`external`、`pipe`、`redirect`、`history`、`logger`、`utils` 八个模块组织。

## 已实现

- 使用 `fork`、`execvp`、`waitpid` 执行外部命令。
- 使用 `pipe`、`dup2` 支持多级管道。
- 支持 `>`、`>>` 和 `2>` 重定向。
- 提供 `xpwd`、`xcd`、`xls`、`xtouch`、`xecho`、`xcat`、`xcp`、`xrm`、`xmv`、`xhistory`、`xtee`、`xjournalctl` 等内建命令。
- 记录命令历史和本地日志；使用 `SIGINT` 处理中断。
- `xcp -r` 与 `xrm -r` 支持递归目录操作。

## 构建与测试

需要 Linux、GCC 和 Make：

```bash
make
make test
./xshell
```

测试覆盖外部命令、输出重定向、多级管道以及递归复制/删除的基本路径。GitHub Actions 会在 Ubuntu 上自动构建和运行冒烟测试。

## 项目边界

这是教学实现，不等同于 Bash。目前不支持引号与转义、变量展开、作业控制、命令替换和输入重定向；管道中的命令按外部程序执行，内建命令不会改变父进程状态。
