#ifndef BUILTIN_H
#define BUILTIN_H

int execute_builtin(char **args, int arg_count);
void xpwd(void);
void xcd(char **args, int arg_count);
void xls(char **args, int arg_count);
void xtouch(char **args, int arg_count);
void xecho(char **args, int arg_count);
void xcat(char **args, int arg_count);
void xcp(char **args, int arg_count);
void xrm(char **args, int arg_count);
void xmv(char **args, int arg_count);
void xtee(char **args, int arg_count);

#endif
