#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "../../util/dynarray.h"

#include "../type.h"
#include "../type_nav.h"
#include "../type_is.h"
#include "../sym.h"
#include "../vla.h"
#include "../gen_asm.h"

#include "../cc1_out_ctx.h"
#include "../cc1.h" /* cc1_fopt */
#include "../fopt.h"

#include "out.h" /* this file defs */
#include "val.h"
#include "ctx.h"
#include "blk.h"
#include "impl.h"

out_val *out_new_blk_addr(out_ctx *octx, out_blk *blk)
{
	type *voidp = type_ptr_to(type_nav_btype(cc1_type_nav, type_void));
	out_blk_mustgen(octx, blk, 0);
	dynarray_add(&octx->mustgen, blk);
	return out_new_lbl(octx, voidp, blk->lbl, OUT_LBL_PIC | OUT_LBL_PICLOCAL);
}

static out_val *out_new_bp_off(out_ctx *octx, long off)
{
	type *voidp = type_ptr_to(type_nav_btype(cc1_type_nav, type_void));
	return v_new_bp3_below(octx, NULL, voidp, off);
}

static type *get_voidpp(void)
{
	return type_ptr_to(type_ptr_to(type_nav_btype(cc1_type_nav, type_void)));
}

out_val *out_new_frame_ptr(out_ctx *octx, int nframes)
{
	type *voidpp = NULL;
	out_val *fp = out_new_bp_off(octx, 0);

	for(; nframes > 1; nframes--){
		if(!voidpp)
			voidpp = get_voidpp();

		assert(fp->retains == 1);
		fp = (out_val *)out_deref(octx, out_change_type(octx, fp, voidpp));
	}

	return fp;
}

const out_val *out_new_return_addr(out_ctx *octx, int nframes)
{
	const out_val *nth_frame = out_new_frame_ptr(octx, nframes);
	const out_val *ret_addr;
	type *voidpp = get_voidpp();

	nth_frame = out_change_type(octx, nth_frame, voidpp);
	ret_addr = out_op(octx,
			op_plus,
			nth_frame,
			out_new_l(octx, type_nav_btype(cc1_type_nav, type_intptr_t), 1));

	return out_deref(octx, ret_addr);
}

out_val *out_new_reg_save_ptr(out_ctx *octx)
{
	return out_new_bp_off(octx, octx->stack_variadic_offset);
}

out_val *out_new_num(out_ctx *octx, type *ty, const numeric *n)
{
	out_val *v = v_new(octx, ty);

	if(n->suffix & VAL_FLOATING){
		v->type = V_CONST_F;
		v->bits.val_f = n->val.f;
	}else{
		v->type = V_CONST_I;
		v->bits.val_i = n->val.i;
	}

	return v;
}

out_val *out_new_l(out_ctx *octx, type *ty, long val)
{
	numeric n;

	n.val.i = val;
	n.suffix = 0;

	return out_new_num(octx, ty, &n);
}

static enum out_pic_type picfilter(enum out_pic_type flags)
{
	return FOPT_PIC(&cc1_fopt) ? flags : OUT_LBL_NOPIC;
}

out_val *out_new_lbl(
		out_ctx *octx, type *ty,
		const char *s,
		enum out_pic_type pic_type)
{
	out_val *v = v_new(octx, ty);

	v->type = V_LBL;
	v->bits.lbl.str = s;
	v->bits.lbl.offset = 0;
	v->bits.lbl.pic_type = picfilter(pic_type);

	return v;
}

out_val *out_new_nan(out_ctx *octx, type *ty)
{
	out_val *v = v_new(octx, ty);

	impl_set_nan(octx, v);

	return v;
}

out_val *out_new_noop(out_ctx *octx)
{
	out_val *v = v_new(octx,
			type_nav_btype(cc1_type_nav, type_void));

	v->type = V_CONST_I;
	return v;
}

const out_val *out_new_overflow(out_ctx *octx, const out_val **eval)
{
	return impl_test_overflow(octx, eval);
}

static const out_val *sym_inline_val(out_ctx *octx, sym *sym)
{
	struct cc1_out_ctx **cc1_octx = cc1_out_ctx(octx);

	if(*cc1_octx){
		dynmap *symmap = (*cc1_octx)->sym_inline_map;

		const out_val *val = dynmap_get(
				struct sym *, const out_val *, symmap, sym);

		if(val)
			return out_val_retain(octx, val);
	}

	return NULL;
}

const out_val *out_new_sym(out_ctx *octx, sym *sym)
{
	/* this function shouldn't be in out/ */

	switch(sym->type){
		case sym_global:
label:
			return gen_decl_addr(octx, sym->decl);

		case sym_local:
		{
			decl *d = sym->decl;

			if(type_is(d->ref, type_func))
				goto label;

			if((d->store & STORE_MASK_STORE) == store_register && d->spel_asm){
				fprintf(stderr, "TODO: %s asm(\"%s\")", decl_to_str(d), d->spel_asm);
				assert(0);
			}

			if(type_is_vla(d->ref, VLA_ANY_DIMENSION))
				return vla_address(d, octx);

			/* fallthru */
		}

		case sym_arg:
		{
			const out_val *v = sym_outval(sym);
			assert(v && "no value for argument");
			octx->used_stack = 1;
			return out_val_retain(octx, v);
		}
	}

	assert(0);
}

const out_val *out_new_sym_val(out_ctx *octx, sym *sym)
{
	if(sym->type == sym_arg){
		/* if an argument is only ever used in rvalue context,
		 * then as an optimisation we don't read from an lvalue
		 * and use that - we inline the rvalue directly */
		const out_val *inlined = sym_inline_val(octx, sym);
		if(inlined)
			return inlined;
	}

	return out_deref(octx, out_new_sym(octx, sym));
}

out_val *out_new_zero(out_ctx *octx, type *ty)
{
	numeric n = { 0 };

	if(type_is_floating(ty))
		n.suffix = VAL_FLOATING; /* sufficient for V_CONST_F */

	return out_new_num(octx, ty, &n);
}
