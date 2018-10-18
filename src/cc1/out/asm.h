#ifndef OUT_ASM_H
#define OUT_ASM_H

enum section_type
{
	SECTION_TEXT,
	SECTION_DATA,
	SECTION_BSS,
	SECTION_RODATA,
	SECTION_CTORS,
	SECTION_DTORS,
	SECTION_DBG_ABBREV,
	SECTION_DBG_INFO,
	SECTION_DBG_LINE
};

#define SECTION_BEGIN "section_begin_"
#define SECTION_END   "section_end_"
#define SECTION_DESC_TEXT "text"
#define SECTION_DESC_DATA "data"
#define SECTION_DESC_BSS "bss"
#define SECTION_DESC_RODATA "rodata"
#define SECTION_DESC_CTORS "ctors"
#define SECTION_DESC_DTORS "dtors"
#define SECTION_DESC_DBG_ABBREV "dbg_abbrev"
#define SECTION_DESC_DBG_INFO "dbg_info"
#define SECTION_DESC_DBG_LINE "dbg_line"

void asm_out_section(enum section_type, const char *fmt, ...);
void asm_out_sectionv(enum section_type t, const char *fmt, va_list l);

int asm_section_empty(enum section_type);

void asm_nam_begin3(enum section_type sec, const char *lbl, unsigned align);

void out_comment_sec(enum section_type sec, const char *fmt, ...);

#ifdef TYPE_H
void asm_out_fp(enum section_type sec, type *ty, floating_t f);
#endif

#ifdef STRINGS_H
void asm_declare_stringlit(enum section_type, const stringlit *);
#endif

#ifdef DECL_H
void asm_declare_decl_init(decl *); /* x: .qword ... */

void asm_declare_constructor(decl *d);
void asm_declare_destructor(decl *d);

void asm_predeclare_extern(decl *d);
void asm_predeclare_global(decl *d);
void asm_predeclare_weak(decl *d);
void asm_predeclare_visibility(decl *d, attribute *);
#endif

/* in impl */
extern const struct asm_type_table
{
	int sz;
	const char *directive;
} asm_type_table[];
#define ASM_TABLE_LEN 4

#ifdef TYPE_H
int         asm_table_lookup(type *);
int         asm_type_size(type *);
const char *asm_type_directive(type *);
#endif

#endif
