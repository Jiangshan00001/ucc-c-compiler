#ifndef GEN_IR_H
#define GEN_IR_H

#define IRTODO(...) ICW("IR TODO: " __VA_ARGS__)

struct strbuf_fixed;

void gen_ir(symtable_global *);

typedef struct irctx irctx;
typedef struct irval irval;

irval *gen_ir_expr(const struct expr *, irctx *);
irval *gen_ir_expr_i1_trunc(const struct expr *, irctx *, irval **const untrunc);
void gen_ir_stmt(const struct stmt *, irctx *);

void gen_ir_comment(irctx *, const char *, ...)
	ucc_printflike(2, 3);

const char *irtype_str(type *, irctx *);
const char *irtype_str_r(struct strbuf_fixed *, type *, irctx *);
const char *irval_str(irval *, irctx *);
const char *irval_str_r(struct strbuf_fixed *, irval *, irctx *);

const char *ir_op_str(enum op_type, int arith_rshift);

int irtype_struct_decl_index(
		struct struct_union_enum_st *su,
		decl *d,
		unsigned **const out_idxs,
		unsigned *const n_out_idx);

/* used for getting first-bitfield type: */
type *irtype_struct_decl_type(
		struct struct_union_enum_st *su,
		struct decl *memb);

/* irvals */
typedef unsigned irid;

irval *irval_from_id(irid);
irval *irval_from_l(type *, integral_t);
irval *irval_from_f(type *, floating_t);
struct sym;
irval *irval_from_sym(irctx *, struct sym *);
irval *irval_from_lbl(irctx *, char *);
irval *irval_from_noop(void);

void irval_free(irval *);
void irval_free_abi(void *);

/* helpers */
void gen_ir_memset(irctx *ctx, irval *v, char ch, size_t len);
void gen_ir_memcpy(irctx *ctx, irval *dest, irval *src, size_t);

#endif