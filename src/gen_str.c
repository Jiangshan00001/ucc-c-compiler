#include <stdio.h>
#include <stdarg.h>

#include "tree.h"
#include "macros.h"
#include "sym.h"


#define PRINT_IF(x, sub, fn) \
	if(x->sub){ \
		idt_printf(#sub ":\n"); \
		indent++; \
		fn(x->sub); \
		indent--; \
	}


static int indent = 0;

void print_tree(tree *t);
void print_expr(expr *e);

void idt_printf(const char *fmt, ...)
{
	va_list l;
	int i;

	for(i = indent; i > 0; i--)
		fputs("  ", stdout);

	va_start(l, fmt);
	vprintf(fmt, l);
	va_end(l);
}

void print_decl(decl *d)
{
	char buf[256];
	int i;

	if(d->spel)
		snprintf(buf, sizeof buf, "spel=\"%s\", ", d->spel);

	idt_printf("{ %sptr_depth=%d, type=%s }\n",
			d->spel ? buf : "",
			d->ptr_depth,
			type_to_str(d->type)
			);

	for(i = 0; d->arraysizes && d->arraysizes[i]; i++){
		idt_printf("array[%d] size:\n", i);
		indent++;
		print_expr(d->arraysizes[i]);
		indent--;
	}
}

void print_sym(sym *s)
{
	idt_printf("sym: type=%s, offset=%d\n", sym_to_str(s->type), s->offset);
	indent++;
	print_decl(s->decl);
	indent--;
}

void print_expr(expr *e)
{
	idt_printf("e->type: %s\n", expr_to_str(e->type));

	idt_printf("vartype:\n");
	indent++;
	print_decl(&e->vartype);
	indent--;

	switch(e->type){
		case expr_identifier:
			idt_printf("identifier: \"%s\"\n", e->spel);
			break;

		case expr_val:
			idt_printf("val: %d\n", e->val);
			break;

		case expr_op:
			idt_printf("op: %s\n", op_to_str(e->op));
			indent++;

			if(e->op == op_deref)
				idt_printf("deref size: %s\n",
						type_to_str(
							e->vartype.ptr_depth ? type_ptr : e->vartype.type));

			PRINT_IF(e, lhs, print_expr);
			PRINT_IF(e, rhs, print_expr);
			indent--;
			break;

		case expr_str:
			idt_printf("str: %s, \"%s\"\n", e->sym->str_lbl, e->spel);
			break;

		case expr_assign:
			idt_printf("assign to %s:\n", e->spel);
			indent++;
			print_expr(e->expr);
			indent--;
			break;

		case expr_funcall:
		{
			expr **iter;

			idt_printf("%s():\n", e->spel);
			indent++;
			if(e->funcargs)
				for(iter = e->funcargs; *iter; iter++){
					idt_printf("arg:\n");
					indent++;
					print_expr(*iter);
					indent--;
				}
			else
				idt_printf("no args\n");
			indent--;
			break;
		}

		case expr_addr:
			idt_printf("&%s\n", e->spel);
			break;

		case expr_sizeof:
			idt_printf("sizeof %s\n", e->spel ? e->spel : type_to_str(e->vartype.type));
			break;

		default:
			idt_printf("\x1b[1;31m%s not handled!\x1b[m\n", expr_to_str(e->type));
			break;
	}
}

void print_tree_flow(tree_flow *t)
{
	idt_printf("flow:\n");
	indent++;
	PRINT_IF(t, for_init,  print_expr);
	PRINT_IF(t, for_while, print_expr);
	PRINT_IF(t, for_inc,   print_expr);
	indent--;
}

void print_tree(tree *t)
{
	idt_printf("t->type: %s\n", stat_to_str(t->type));

	PRINT_IF(t, lhs,  print_tree);
	PRINT_IF(t, rhs,  print_tree);
	PRINT_IF(t, expr, print_expr);

	if(t->decls){
		decl **iter;

		idt_printf("decls:\n");
		for(iter = t->decls; *iter; iter++){
			indent++;
			print_decl(*iter);
			indent--;
		}
	}

	if(t->codes){
		tree **iter;

		idt_printf("code(s):\n");
		for(iter = t->codes; *iter; iter++){
			indent++;
			print_tree(*iter);
			indent--;
		}
	}

	if(t->flow){
		indent++;
		print_tree_flow(t->flow);
		indent--;
	}
}

void gen_str(function *f)
{
	sym *s;

	idt_printf("function: decl:\n");
	indent++;

	print_decl(f->func_decl);

	if(f->symtab){
		idt_printf("symtable:\n");
		for(s = f->symtab->first; s; s = s->next){
			indent++;
			print_sym(s);
			indent--;
		}
	}

	PRINT_IF(f, code, print_tree)

	indent--;
}
