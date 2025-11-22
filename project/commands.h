#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_cp(char *src, char *dest);
void cmd_mv(char *src, char *dest);
void cmd_ln(char *src, char *dest);
void cmd_grep(char *pattern, char *filename);

#endif