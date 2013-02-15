/* i386.c  --  i386 specific code
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

load_num(stream, num) {
    fprintf(stream, "\tMOVL\t$%d, %%eax\n", num);
}

load_chr(stream, chr) {
    fprintf(stream, "\tMOVL\t$%s, %%eax\n", chr);
}

load_str(stream, str, clabel) {
    fprintf(stream, ".data .LC%d: .string %s\n.text\tMOVL\t$.LC%d, %%eax\n",
            clabel, str, clabel);
}

load_local(stream, offset, need_lval) {
    fprintf(stream, "\t%s\t%d(%%ebp), %%eax)\n", 
            need_lval ? "LEAL" : "MOVL", offset);
}

load_symbol(stream, name, need_lval) {
    fprintf(stream, "\tMOVL\t%s%s, %%eax\n", need_lval ? "$" : "", name);
}

asm_push(stream) {
    fputs("\tPUSH\t%eax\n", stream);
}

arith_neg(stream) {
    fputs("\tNEGL\t%eax\n", stream);
}

bit_not(stream) {
    fputs("\tNOTL\t%%eax\n", stream);
}

logic_not(stream) {
    fputs("\tTESTL\t%eax, %eax\n\tSETZ\t%al\n\tMOVZBL\t%al, %eax\n", stream);
}

dereference(stream, need_lval) {
    if (!need_lval) fputs("\tMOVL\t(%eax), %eax\n", stream);
}

increment(stream) {
    fputs("\tINCL\t(%eax)\n", stream);
}

decrement(stream) {
    fputs("\tDECL\t(%eax)\n", stream);
}

postfix_inc(stream) {
    fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tINCL\t(%ecx)\n", stream);
}

postfix_dec(stream) {
    fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tDECL\t(%ecx)\n", stream);
}

static
start_binop(stream, is_assign, is_sym) {
    if (is_sym && !is_assign) fputs("\tPOP\t%ecx\n", stream);
    else fputs("\tMOVL\t%eax, %ecx\n\tPOP\t%eax\n", stream);
}

pop_mult(stream, is_assign) {
    if (is_assign)
        fputs("\tMOVL\t(%esp), %ecx\n", stream);
    else
        fputs("\tPOP\t%ecx\n", stream);
    fputs("\tIMULL\t%ecx\n", stream);
    if (is_assign)
        fputs("\tPOP\t%ecx\n\tMOVL\t%eax, (%ecx)\n\tMOVL\t%ecx, %eax\n", 
              stream);
}

pop_div(stream, is_assign) {
    if (is_assign)
        fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%esp), %eax\n", stream);
    else
        fputs("\tMOVL\t%eax, %ecx\n\tPOP\t%eax\n", stream);
    fputs("\tXORL\t%edx, %edx\n\tIDIVL\t%ecx\n", stream);
    if (is_assign)
        fputs("\tPOP\t%ecx\nMOVL\t%eax, (%ecx)\nMOVL\t%ecx, %eax\n", stream);
}

pop_mod(stream, is_assign) {
    if (is_assign)
        fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%esp), %eax\n", stream);
    else
        fputs("\tMOVL\t%eax, %ecx\n\tPOP\t%eax\n", stream);
    fputs("\tXORL\t%edx, %edx\n\tIDIVL\t%ecx\n", stream);
    if (is_assign)
        fputs("\tPOP\t%eax\nMOVL\t%edx, (%eax)\n", stream);
    else
        fputs("\tMOVL\t%edx, %eax\n", stream);
}

static
pop_shift(stream, mnemonic, is_assign) {
    start_binop(stream, is_assign, 0);
    fprintf(stream, "\t%s\t%s\n", mnemonic, is_assign ? "(%eax)" : "%eax");
}

pop_lshift(stream, is_assign) { pop_shift(stream, "SALL", is_assign); }
pop_rshift(stream, is_assign) { pop_shift(stream, "SARL", is_assign); }

static
pop_rel(stream, mnemonic) {
    fprintf(stream, "\tPOP\t%%ecx\n\tCMPL\t%%eax, %%ecx\n");
    fprintf(stream, "\t%s\t%%al\n\tMOVZBL\t%%al, %%eax\n", mnemonic);
}

pop_gt(stream) { pop_rel(stream, "SETG");  }
pop_lt(stream) { pop_rel(stream, "SETL");  }
pop_ge(stream) { pop_rel(stream, "SETGE"); }
pop_le(stream) { pop_rel(stream, "SETLE"); }
pop_eq(stream) { pop_rel(stream, "SETE");  }
pop_ne(stream) { pop_rel(stream, "SETNE"); }

static
pop_binop(stream, mnemonic, is_assign, is_sym) {
    start_binop(stream, is_assign, is_sym);
    fprintf(stream, "\t%s\t%%ecx, %s\n", mnemonic, 
            is_assign ? "(%eax)" : "%eax");
}

pop_add   (stream, is_assign) { pop_binop(stream, "ADDL", is_assign, 1); }
pop_sub   (stream, is_assign) { pop_binop(stream, "SUBL", is_assign, 0); }
pop_bitand(stream, is_assign) { pop_binop(stream, "ANDL", is_assign, 1); }
pop_bitor (stream, is_assign) { pop_binop(stream, "ORL",  is_assign, 1);  }
pop_bitxor(stream, is_assign) { pop_binop(stream, "XORL", is_assign, 1); }

pop_subscr(stream, need_lval) {
    fputs("\tMOVB\t$2, %cl\n\tSHLL\t%eax\n", stream);
    pop_add(stream, 0);
    if (!need_lval) dereference();
}

pop_assign(stream) {
    start_binop(stream, 1, 0);
    fputs("\tMOVL\t%ecx,(%eax)\n", stream);
}

asm_call(stream, fn_name, nargs) {
    fprintf(stream, "\tCALL\t%s\n", fn_name);
    if (nargs)
        fprintf(stream, "\tADDL\t$%d, %%esp\n", nargs*4);
}

static
cond_branch(stream, mnemonic, label_num) {
    fprintf(stream, "\tTESTL\t%%eax, %%eax\n\t%s\t.L%d\n", 
            mnemonic, label_num);
}

branch_ifz(stream, label_num) {
    cond_branch(stream, "JZ", label_num);
}

branch_ifnz(stream, label_num) {
    cond_branch(stream, "JNZ", label_num);
}

branch(stream, label_num) {
    fprintf(stream, "\tJMP\t.L%d\n", label_num);
}    

emit_label(stream, label_num) {
    fprintf(stream, ".L%d:\n", label_num);
}

cast_bool(stream) {
    fputs("\tTESTL\t%eax, %eax\n\tSETNZ\t%al\n\tMOVZBL\t%al, %eax\n", stream);
}
