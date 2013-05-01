#include <string.h>

#include "ops.h"
#include "stmt_expr.h"

const char *str_stmt_expr()
{
	return "expr";
}

void fold_stmt_expr(stmt *s)
{
	FOLD_EXPR(s->expr, s->symtab);
	if(!s->freestanding && !s->expr->freestanding && !type_ref_is_void(s->expr->tree_type))
		cc1_warn_at(&s->expr->where, 0, 1, WARN_UNUSED_EXPR,
				"unused expression (%s)", s->expr->f_str());
}

void gen_stmt_expr(stmt *s)
{
	const int pre_vcount = out_vcount();

	gen_expr(s->expr, s->symtab);

	if(!s->expr_no_pop){
		out_pop(); /* cancel the implicit push from gen_expr() above */
		out_comment("end of %s-stmt", s->f_str());

		UCC_ASSERT(out_vcount() == pre_vcount,
				"vcount changed over %s statement (%d -> %d)",
				s->expr->f_str(),
				out_vcount(), pre_vcount);
	}
}

static int expr_passable(stmt *s)
{
	/*
	 * TODO: ({}) - return inside?
	 * if we have a funcall marked noreturn, we're not passable
	 */
	if(expr_kind(s->expr, funcall))
		return expr_func_passable(s->expr);

	if(expr_kind(s->expr, stmt))
		return fold_passable(s->expr->code);

	return 1;
}

void mutate_stmt_expr(stmt *s)
{
	s->f_passable = expr_passable;
}
