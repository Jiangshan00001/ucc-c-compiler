#ifndef CTX_H
#define CTX_H

typedef struct out_val_list out_val_list;

struct out_ctx
{
	out_blk *current_blk;

	struct out_val_list
	{
		out_val val;
		struct out_val_list *next, *prev;
	} *val_head, *val_tail;

	int stack_sz, stack_local_offset, stack_variadic_offset;

	/* we won't reserve it more than 255 times
	 * XXX: FIXME: hardcoded 20
	 */
	unsigned char reserved_regs[20];
};

#endif
