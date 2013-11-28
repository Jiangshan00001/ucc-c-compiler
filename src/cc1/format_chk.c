#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "../util/where.h"
#include "../util/util.h"
#include "../util/dynarray.h"

#include "data_structs.h"
#include "expr.h"
#include "decl.h"
#include "const.h"
#include "funcargs.h"

#include "format_chk.h"

enum printf_attr
{
	printf_attr_long = 1 << 0
};

static void format_check_printf_1(char fmt, type_ref *const t_in,
		where *w, enum printf_attr attr)
{
	int allow_long = 0;
	char expected[TYPE_STATIC_BUFSIZ];

	expected[0] = '\0';

	switch(fmt){
		enum type_primitive prim;
		type_ref *tt;

		case 's': prim = type_char; goto ptr;
		case 'p': prim = type_void; goto ptr;
		case 'n': prim = type_int;  goto ptr;
ptr:
			tt = type_ref_is_type(type_ref_is_ptr(t_in), prim);
			if(!tt){
				snprintf(expected, sizeof expected,
						"'%s *'", type_primitive_to_str(prim));
			}
			break;

		case 'x':
		case 'X':
		case 'u':
		case 'o':
			/* unsigned ... */
		case '*':
		case 'c':
		case 'd':
		case 'i':
			allow_long = 1;
			if(!type_ref_is_integral(t_in))
				strcpy(expected, "integral");
			if((attr & printf_attr_long) && !type_ref_is_type(t_in, type_long))
				strcpy(expected, "'long'");
			break;

		case 'e':
		case 'E':
		case 'f':
		case 'F':
		case 'g':
		case 'G':
		case 'a':
		case 'A':
			if(!type_ref_is_floating(t_in))
				strcpy(expected, "'double'");
			break;

		default:
			warn_at(w, "unknown conversion character 0x%x", fmt);
			return;
	}

	if(!*expected && !allow_long && attr & printf_attr_long)
		strcpy(expected, "'long'");

	if(*expected){
		warn_at(w, "format %%%s%c expects %s argument (got %s)",
				attr & printf_attr_long ? "l" : "", fmt,
				expected, type_ref_to_str(t_in));
	}
}

static void format_check_printf_str(
		expr **args,
		const char *fmt, const int len,
		int var_idx, where *w)
{
	int n_arg = 0;
	int i;

	for(i = 0; i < len && fmt[i];){
		if(fmt[i++] == '%'){
			int fin;
			enum printf_attr attr;
			expr *e;

			if(i == len){
				where strloc = *w;
				strloc.chr += i + 1; /* +1 since we start on the '(' */
				warn_at(&strloc, "incomplete format specifier");
				return;
			}

			if(fmt[i] == '%'){
				i++;
				continue;
			}

recheck:
			attr = 0;
			fin = 0;
			do switch(fmt[i]){
				 case 'l':
					/* TODO: multiple check.. */
					attr |= printf_attr_long;

				case '1': case '2': case '3':
				case '4': case '5': case '6':
				case '7': case '8': case '9':

				case '0': case '#': case '-':
				case ' ': case '+': case '.':

				case 'h': case 'L':
					i++;
					break;
				default:
					fin = 1;
			}while(!fin);

			/* don't check for format(printf, $x, 0) */
			if(var_idx != -1){
				e = args[var_idx + n_arg++];

				if(!e){
					warn_at(w, "too few arguments for format (%%%c)", fmt[i]);
					break;
				}

				format_check_printf_1(fmt[i], e->tree_type, &e->where, attr);
			}

			if(fmt[i] == '*'){
				i++;
				goto recheck;
			}
		}
	}

	if(var_idx != -1 && (!fmt[i] || i == len) && args[var_idx + n_arg])
		warn_at(w, "too many arguments for format");
}

static void format_check_printf(
		expr *str_arg,
		expr **args,
		unsigned var_idx,
		where *w)
{
	stringlit *fmt_str;
	consty k;

	const_fold(str_arg, &k);

	switch(k.type){
		case CONST_NO:
		case CONST_NEED_ADDR:
			/* check for the common case printf(x?"":"", ...) */
			if(expr_kind(str_arg, if)){

				format_check_printf(
						str_arg->lhs ? str_arg->lhs : str_arg->expr,
						args, var_idx, w);

				format_check_printf(str_arg->rhs, args, var_idx, w);
				return;
			}
			goto not_string;

		case CONST_VAL:
			if(k.bits.iv.val == 0)
				return; /* printf(NULL, ...) */
			/* fall */

		case CONST_ADDR:
not_string:
			warn_at(&str_arg->where, "format argument isn't a string constant");
			return;

		case CONST_STRK:
			fmt_str = k.bits.str->lit;
			break;
	}

	{
		const char *fmt = fmt_str->str;
		const int   len = fmt_str->len - 1;

		if(len <= 0)
			;
		else if(k.offset >= len)
			warn_at(&str_arg->where, "undefined printf-format argument");
		else
			format_check_printf_str(args, fmt + k.offset, len, var_idx, w);
	}
}

void format_check_call(
		where *w, type_ref *ref,
		expr **args, const int variadic)
{
	decl_attr *attr = type_attr_present(ref, attr_format);
	int n, fmt_idx, var_idx;

	if(!attr || !attr->attr_extra.format.valid || !variadic)
		return;

	fmt_idx = attr->attr_extra.format.fmt_idx;
	var_idx = attr->attr_extra.format.var_idx;

	n = dynarray_count(args);

	/* do bounds checks here, but no warnings
	 * warnings are on the decl
	 *
	 * if var_idx is zero we only check the format string,
	 * not the arguments
	 */
	if(fmt_idx >= n
	|| var_idx > n
	|| (var_idx > -1 && var_idx <= fmt_idx))
	{
		return;
	}

	switch(attr->attr_extra.format.fmt_func){
		case attr_fmt_printf:
			format_check_printf(args[fmt_idx], args, var_idx, w);
			break;

		case attr_fmt_scanf:
			ICW("scanf check");
			break;
	}
}

void format_check_decl(decl *d, decl_attr *da)
{
	type_ref *r_func = type_ref_is_func_or_block(d->ref);
	funcargs *fargs;
	int fmt_idx, var_idx, nargs;

	assert(r_func);
	fargs = r_func->bits.func.args;

	if(!fargs->variadic){
		warn_at(&da->where, "variadic function required for format attribute");
		return;
	}

	nargs = dynarray_count(fargs->arglist);
	fmt_idx = da->attr_extra.format.fmt_idx;
	var_idx = da->attr_extra.format.var_idx;

	/* format string index must be < nargs */
	if(fmt_idx >= nargs){
		warn_at(&da->where, "format argument out of bounds (%d >= %d)",
				fmt_idx, nargs);
		return;
	}

	if(var_idx != -1){
		/* variadic index must be nargs */
		if(var_idx != nargs){
			warn_at(&da->where, "variadic argument out of bounds (should be %d)",
					nargs + 1); /* +1 to get to the "..." index as 1-based */
			return;
		}

		assert(var_idx > fmt_idx);
	}

	da->attr_extra.format.valid = 1;
}
