#ifndef ASM_H
#define ASM_H

enum section_type
{
	SECTION_TEXT,
	SECTION_DATA,
	SECTION_BSS,
	SECTION_DBG_ABBREV,
	SECTION_DBG_INFO,
	SECTION_DBG_LINE,
	NUM_SECTIONS
};

#define SECTION_BEGIN ".Lsection_begin_"
#define SECTION_END   ".Lsection_end_"
#define QUOTE_(...) #__VA_ARGS__
#define QUOTE(y) QUOTE_(y)

extern FILE *cc_out[NUM_SECTIONS];
extern FILE *cc1_out;

void asm_out_section(enum section_type, const char *fmt, ...);
void asm_out_sectionv(enum section_type t, const char *fmt, va_list l);

void asm_nam_begin3(enum section_type sec, const char *lbl, unsigned align);

#ifdef TYPE_REF_H
void asm_out_fp(enum section_type sec, type_ref *ty, floating_t f);
#endif

#ifdef STRINGS_H
void asm_declare_stringlit(enum section_type, const stringlit *);
#endif

#ifdef DECL_H
void asm_declare_decl_init(enum section_type, decl *); /* x: .qword ... */

void asm_predeclare_extern(decl *d);
void asm_predeclare_global(decl *d);
#endif

/* in impl */
extern const struct asm_type_table
{
	int sz;
	const char *directive;
} asm_type_table[];
#define ASM_TABLE_LEN 4

#ifdef TYPE_REF_H
int         asm_table_lookup(type_ref *);
int         asm_type_size(type_ref *);
const char *asm_type_directive(type_ref *);
#endif

#endif
