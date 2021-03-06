#ifndef TOKCONV_H
#define TOKCONV_H

#include "op.h"
#include "btype.h"
#include "decl.h"

enum op_type curtok_to_op(void);

enum type_primitive curtok_to_type_primitive(void);
enum type_qualifier curtok_to_type_qualifier(void);
enum decl_storage   curtok_to_decl_storage(void);

int eat(enum token t); /* returns 1 on success */
int eat2(enum token t, int die);
void uneat(enum token t);
int accept(enum token t);
int accept_where(enum token t, where *);

#define EAT(t) eat(t)
#define DUMMY_IDENTIFIER "?"

int curtok_is_type_primitive(void);
int curtok_is_type_qual(void);
int curtok_is_decl_store(void);

int curtok_in_list(va_list l);

/* callers should check .is_wide matches and if .length is not wanted,
 * that the string is nul-terminated */
struct cstring *token_get_current_str(where *);

enum op_type curtok_to_compound_op(void);
int          curtok_is_compound_assignment(void);

char *token_to_str(enum token t);
char *eat_curtok_to_identifier(int *alloc, where *loc); /* e.g. token_const -> "const" */

#endif
