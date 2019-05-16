#include "ops.h"
#include "stmt_label.h"
#include "../label.h"
#include "../out/lbl.h"

const char *str_stmt_label(void)
{
	return "label";
}

void fold_stmt_label(stmt *s)
{
	label *l = symtab_label_find_or_new(
			s->symtab, s->bits.lbl.spel, &s->where);

	/* update its where */
	memcpy_safe(&l->where, &s->where);
	/* update its scope */
	l->scope = s->symtab;
	/* update code the label uses */
	l->next_stmt = s;

	if(l->complete){
		warn_at_print_error(&s->where, "duplicate label '%s'", s->bits.lbl.spel);
		fold_had_error = 1;
	}else{
		l->complete = 1;
	}

	s->bits.lbl.label = l;

	l->unused = s->bits.lbl.unused;

	fold_stmt(stmt_label_sub(s)); /* compound */
}

void gen_stmt_label_m1(const stmt *s, out_ctx *octx, int m1)
{
	out_blk *thisblk = label_getblk(s->bits.lbl.label, octx);

	if(s->bits.lbl.label->mustgen_spel)
		out_blk_mustgen(octx, thisblk, s->bits.lbl.label->mustgen_spel);

	/* explicit fall through */
	out_ctrl_transfer_make_current(octx, thisblk);

	if(m1)
		return;
	gen_stmt(stmt_label_sub(s), octx);
}

void gen_stmt_label(const stmt *s, out_ctx *octx)
{
	gen_stmt_label_m1(s, octx, 0);
}

void dump_stmt_label(const stmt *s, dump *ctx)
{
	dump_desc_stmt_newline(ctx, "label", s, 0);
	dump_printf_indent(ctx, 0, " %s\n", s->bits.lbl.spel);

	dump_inc(ctx);
	dump_stmt(stmt_label_sub(s), ctx);
	dump_dec(ctx);
}

void style_stmt_label(const stmt *s, out_ctx *octx)
{
	stylef("\n%s: ", s->bits.lbl.spel);
	gen_stmt(stmt_label_sub(s), octx);
}

int label_passable(stmt *s)
{
	return fold_passable(stmt_label_sub(s));
}

void init_stmt_label(stmt *s)
{
	s->f_passable = label_passable;
}
