#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../../util/alloc.h"

#include "../type.h"
#include "../type_nav.h"

#include "../macros.h"

#include "val.h"
#include "ctx.h"
#include "virt.h"

#include "../op.h"
#include "asm.h"
#include "impl.h"
#include "out.h" /* retain/release prototypes */

#include "../cc1.h" /* cc1_type_nav */

const char *v_store_to_str(enum out_val_store store)
{
	switch(store){
		CASE_STR(V_CONST_I);
		CASE_STR(V_REG);
		CASE_STR(V_REG_SPILT);
		CASE_STR(V_LBL);
		CASE_STR(V_CONST_F);
		CASE_STR(V_FLAG);
	}
	return NULL;
}

static void v_register(out_ctx *octx, out_val_list *l)
{
	if(octx->val_head)
		assert(!octx->val_head->prev);

	/* double link */
	l->next = octx->val_head;
	if(octx->val_head)
		octx->val_head->prev = l;

	/* store in val_head */
	octx->val_head = l;

	if(!octx->val_tail)
		octx->val_tail = l->next;
}

static void v_init(out_val *v, type *ty)
{
	v->retains = 1;
	v->t = ty;
}

out_val *v_new(out_ctx *octx, type *ty)
{
	out_val_list *l = umalloc(sizeof *l);
	out_val *v = &l->val;

	v_init(v, ty);
	v_register(octx, l);

	return v;
}

static out_val *v_dup(out_ctx *octx, const out_val *from, type *ty)
{
	switch(from->type){
		case V_CONST_I:
		case V_CONST_F:
		case V_LBL:
copy:
		{
			out_val *v = v_new(octx, ty);
			out_val_overwrite(v, from);
			v->t = ty;
			return v;
		}

		case V_FLAG:
		{
			const out_val *dup = v_to_reg(octx, from);
			out_val_overwrite((out_val *)from, dup); /* replace flag */
			/* fall */
			from = dup;
		}

		case V_REG_SPILT:
		case V_REG:
		{
			struct vreg r;
			out_val *new;

			if(impl_reg_frame_const(&from->bits.regoff.reg))
				goto copy;

			/* copy to a new register */
			v_unused_reg(
						octx, /*stack backup:*/1,
						from->bits.regoff.reg.is_float, &r);

			/* dup */
			impl_reg_cp(octx, from, &r);

			new = v_new(octx, ty);
			new->type = from->type;
			new->bits.regoff.reg = r;
			new->bits.regoff.offset = from->bits.regoff.offset;

			return new;
		}
	}

	assert(0);
}

static out_val *v_reuse(out_ctx *octx, const out_val *from, type *ty)
{
	out_val *mut;

	if(!from || from->retains > 1){
		if(from)
			out_val_consume(octx, from);

		return v_new(octx, ty);
	}

	assert(from->retains == 1);
	mut = (out_val *)from;

	mut->t = ty;
	return mut; /* reuse */
}

out_val *v_dup_or_reuse(out_ctx *octx, const out_val *from, type *ty)
{
	assert(from);

	if(from->retains > 1){
		out_val *r = v_dup(octx, from, ty);
		out_val_consume(octx, from);
		return r;
	}

	return v_reuse(octx, from, ty);
}

out_val *v_new_flag(
		out_ctx *octx, const out_val *from,
		enum flag_cmp cmp, enum flag_mod mod)
{
	out_val *v = v_reuse(octx, from,
			type_nav_btype(cc1_type_nav, type__Bool));

	v->type = V_FLAG;
	v->bits.flag.cmp = cmp;
	v->bits.flag.mods = mod;
	return v;
}

out_val *v_new_reg(
		out_ctx *octx, const out_val *from,
		type *ty, const struct vreg *reg)
{
	/* reg may alias from->bits... */
	const struct vreg savedreg = *reg;
	out_val *v = v_reuse(octx, from, ty);
	v->type = V_REG;
	v->bits.regoff.offset = 0;
	memcpy_safe(&v->bits.regoff.reg, &savedreg);
	return v;
}

out_val *v_new_sp(out_ctx *octx, const out_val *from)
{
	struct vreg r;

	r.is_float = 0;
	r.idx = REG_SP;

	return v_new_reg(octx, from, type_nav_voidptr(cc1_type_nav), &r);
}

out_val *v_new_sp3(
		out_ctx *octx, const out_val *from,
		type *ty, long stack_pos)
{
	out_val *v = v_new_sp(octx, from);
	v->t = ty;
	v->bits.regoff.offset = stack_pos;
	return v;
}

out_val *v_new_bp3(
		out_ctx *octx, const out_val *from,
		type *ty, long stack_pos)
{
	out_val *v = v_new_sp3(octx, from, ty, stack_pos);
	v->bits.regoff.reg.idx = REG_BP;
	return v;
}

const out_val *out_val_release(out_ctx *octx, const out_val *v)
{
	out_val *mut = (out_val *)v;
	(void)octx;
	assert(mut->retains > 0 && "double release");
	if(--mut->retains == 0)
		return NULL;
	return mut;
}

const out_val *out_val_retain(out_ctx *octx, const out_val *v)
{
	(void)octx;
	((out_val *)v)->retains++;
	return v;
}

void out_val_overwrite(out_val *d, const out_val *s)
{
	/* don't copy .retains */
	d->type = s->type;
	d->t = s->t;
	d->bitfield = s->bitfield;
	d->bits = s->bits;
}

int vreg_eq(const struct vreg *a, const struct vreg *b)
{
	return a->idx == b->idx && a->is_float == b->is_float;
}
