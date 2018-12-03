#ifndef UTIL_STR_H
#define UTIL_STR_H

char *str_quotefin2(char *s, char q);

char *str_spc_skip(const char *);
char *str_quotefin(char *);
char *char_quotefin(char *);

int str_endswith(const char *, const char *);

#endif
