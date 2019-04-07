#ifndef SANITIZE_H
#define SANITIZE_H

void sanitize_fail(out_ctx *, const char *desc);

void sanitize_boundscheck(
		expr *elhs, expr *erhs,
		out_ctx *octx,
		const out_val *lhs, const out_val *rhs);

void sanitize_vlacheck(const out_val *vla_sz, type *sz_ty, out_ctx *);

void sanitize_shift(
		expr *elhs, expr *erhs,
		enum op_type,
		out_ctx *octx,
		const out_val **lhs, const out_val **rhs);

void sanitize_nonnull_args(symtable *, out_ctx *);

void sanitize_divide(const out_val *lhs, const out_val *rhs, type *, out_ctx *);

#endif
