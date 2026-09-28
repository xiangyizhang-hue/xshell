#ifndef REDIRECT_H
#define REDIRECT_H

int handle_redirection(char **args, int *arg_count);
void restore_stdout(void);
void restore_stderr(void);

#endif
