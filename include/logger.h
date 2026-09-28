#ifndef LOGGER_H
#define LOGGER_H

void init_logger(void);
void log_command(const char *command);
void log_error(const char *error_msg);
void xjournalctl(void);

#endif
