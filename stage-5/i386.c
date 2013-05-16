/* i386.c  --  i386 specific code
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static
sz_suffix(sz) {
    extern compat_flag;
    if (sz == 4) return 'L';

    /* Stage 4 treated everything as an int. */
    if (compat_flag) 
        error("Operand size has changed since stage-4");

    if (sz == 2) return 'W';
    else if (sz == 1) return 'B';
    int_error("Unexpected size operand to instruction: %d", sz);
}

static
sz_accum(sz) {
    if (sz == 4) return "%eax";
    else if (sz == 2) return "%ax";
    else if (sz == 1) return "%al";
    int_error("Unexpected size register requested: %d", sz);
}

static
sz_aux_reg(sz) {
    if (sz == 4) return "%ecx";
    else if (sz == 2) return "%cx";
    else if (sz == 1) return "%cl";
    int_error("Unexpected size register requested: %d", sz);
}

load_num(stream, num) {
    fprintf(stream, "\tMOVL\t$%d, %%eax\n", num);
}

load_chr(stream, chr) {
    fprintf(stream, "\tMOVL\t$%s, %%eax\n", chr);
}

load_str(stream, str, clabel) {
    fprintf(stream, 
            ".data\n.LC%d:\n\t.string %s\n.text\n\tMOVL\t$.LC%d, %%eax\n",
            clabel, str, clabel);
}

load_local(stream, offset, need_addr) {
    fprintf(stream, "\t%s\t%d(%%ebp), %%eax\n", 
            need_addr ? "LEA" : "MOVL", offset);
}

load_symbol(stream, name, need_addr) {
    fprintf(stream, "\tMOVL\t%s%s, %%eax\n", need_addr ? "$" : "", name);
}

save_local(stream, offset) {
    fprintf(stream, "\tMOVL\t%%eax, %d(%%ebp)\n", offset);
}

asm_push(stream) {
    fputs("\tPUSHL\t%eax\n", stream);
}

arith_neg(stream) {
    fputs("\tNEGL\t%eax\n", stream);
}

bit_not(stream) {
    fputs("\tNOTL\t%eax\n", stream);
}

logic_not(stream) {
    fputs("\tTESTL\t%eax, %eax\n\tSETZ\t%al\n\tMOVZBL\t%al, %eax\n", stream);
}

dereference(stream, need_lval) {
    if (!need_lval) fputs("\tMOVL\t(%eax), %eax\n", stream);
}

increment(stream) {
    fputs("\tINCL\t(%eax)\n\tMOVL\t(%eax), %eax\n", stream);
}

decrement(stream) {
    fputs("\tDECL\t(%eax)\n\tMOVL\t(%eax), %eax\n", stream);
}

postfix_inc(stream) {
    fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tINCL\t(%ecx)\n", stream);
}

postfix_dec(stream) {
    fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tDECL\t(%ecx)\n", stream);
}

static
start_binop(stream, is_assign, is_sym) {
    if (is_sym && !is_assign) fputs("\tPOPL\t%ecx\n", stream);
    else fputs("\tMOVL\t%eax, %ecx\n\tPOPL\t%eax\n", stream);
}

pop_mult(stream, is_assign, is_unsigned) {
    fputs("\tPOPL\t%ecx\n", stream);
    fprintf(stream, "\t%s%c\t%s\n", is_unsigned ? "MUL" : "IMUL", 'L',
            is_assign ? "(%ecx)" : "%ecx");
    if (is_assign)
        fputs("\tMOVL\t%eax, (%ecx)\n\tMOVL\t%ecx, %eax\n", stream);
}

static
common_div(stream, is_assign, is_unsigned) {
    if (is_assign)
        fputs("\tMOVL\t%eax, %ecx\n\tMOVL\t(%esp), %eax\nMOVL\t(%eax), %eax\n", 
              stream);
    else
        fputs("\tMOVL\t%eax, %ecx\n\tPOPL\t%eax\n", stream);
    fputs("\tXORL\t%edx, %edx\n", stream);
    fprintf(stream, "\t%s%c\t%%ecx\n", is_unsigned ? "DIV" : "IDIV", 'L');
}

pop_div(stream, is_assign, is_unsigned) {
    common_div(stream, is_assign, is_unsigned);
    if (is_assign)
        fputs("\tPOPL\t%ecx\nMOVL\t%eax, (%ecx)\nMOVL\t%ecx, %eax\n", stream);
}

pop_mod(stream, is_assign, is_unsigned) {
    common_div(stream, is_assign, is_unsigned);
    if (is_assign)
        fputs("\tPOPL\t%eax\nMOVL\t%edx, (%eax)\n", stream);
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
    fprintf(stream, "\tPOPL\t%%ecx\n\tCMPL\t%%eax, %%ecx\n");
    fprintf(stream, "\t%s\t%%al\n\tMOVZBL\t%%al, %%eax\n", mnemonic);
}

pop_gt(stream) { pop_rel(stream, "SETG");  }
pop_lt(stream) { pop_rel(stream, "SETL");  }
pop_ge(stream) { pop_rel(stream, "SETGE"); }
pop_le(stream) { pop_rel(stream, "SETLE"); }
pop_eq(stream) { pop_rel(stream, "SETE");  }
pop_ne(stream) { pop_rel(stream, "SETNE"); }

static
pop_binop(stream, mnemonic, is_assign, is_sym, sz) {
    start_binop(stream, is_assign, is_sym);
    fprintf(stream, "\t%s%c\t%s, %s\n", mnemonic, sz_suffix(sz), 
            sz_aux_reg(sz), is_assign ? "(%eax)" : sz_accum(sz) );
}

pop_add(stream, is_assign, sz) {
    pop_binop(stream, "ADD", is_assign, 1, sz);
}
pop_sub(stream, is_assign, sz) {
    pop_binop(stream, "SUB", is_assign, 0, sz);
}
pop_bitand(stream, is_assign, sz) {
    pop_binop(stream, "AND", is_assign, 1, sz);
}
pop_bitor(stream, is_assign, sz) {
    pop_binop(stream, "OR",  is_assign, 1, sz);
}
pop_bitxor(stream, is_assign, sz) {
    pop_binop(stream, "XOR", is_assign, 1, sz);
}

static
ilog2(i) {
    auto l = 0;
    while (i >>= 1) ++l;
    return l;
}

pop_subscr(stream, elt_size, need_lval) {
    if ( elt_size == 1 )
        ;
    else if ( (elt_size & (elt_size-1)) == 0 )
        /* If the size is a power of two, then use SHLL for speed */
        fprintf(stream, "\tMOVB\t$%d, %%cl\n\tSHLL\t%%eax\n", ilog2(elt_size));
    else
        fprintf(stream, "\tMOVL\t$%d, %%ecx\n\tMULL\t%%ecx\n", elt_size);
    pop_add(stream, 0, 4);
    dereference(stream, need_lval);
}

pop_assign(stream, sz) {
    start_binop(stream, 1, 0);
    fprintf(stream, "\tMOV%c\t%s, (%%eax)\n", 
            sz_suffix(sz), sz_aux_reg(sz));
}


load_zero(stream) {
    fputs("\tXORL\t%eax, %eax\n", stream);
}

alloc_stack(stream, sz) {
    if (sz)
        fprintf(stream, "\tSUBL\t$%d, %%esp\n", sz);
}

clear_stack(stream, sz) {
    if (sz)
        fprintf(stream, "\tADDL\t$%d, %%esp\n", sz);
}

asm_call(stream, fn_name, cleanup_sz) {
    fprintf(stream, "\tCALL\t%s\n", fn_name);
    clear_stack( stream, cleanup_sz );
}

call_ptr(stream, cleanup_sz) {
    fputs("\tCALL\t*%eax\n", stream);
    clear_stack( stream, cleanup_sz );
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

branch_eq_n(stream, n, label_num) {
    fprintf(stream, "\tCMPL\t$%d, %%eax\n\tJE\t.L%d\n", n, label_num);
}

emit_label(stream, label_num) {
    fprintf(stream, ".L%d:\n", label_num);
}

cast_bool(stream) {
    fputs("\tTESTL\t%eax, %eax\n\tSETNZ\t%al\n\tMOVZBL\t%al, %eax\n", stream);
}

globl_decl(stream, name) {
    fprintf(stream, ".globl\t%s\n", name);
}

local_decl(stream, name) {
    fprintf(stream, ".local\t%s\n", name);
}

prolog(stream, name, frame_sz) {
    fprintf(stream, ".text\n%s:\n\tPUSHL\t%%ebp\n\tMOVL\t%%esp, %%ebp\n", name);
    alloc_stack(stream, frame_sz);
}

epilog(stream, frame_sz) {
    clear_stack(stream, frame_sz);
    fputs("\tLEAVE\n\tRET\n\n", stream);
}

data_decl(stream, name) {
    fprintf(stream, ".data\n%s:\n", name);
}

int_decl_n(stream, num) {
    fprintf(stream, "\t.int %d\n", num);
}

int_decl_s(stream, str) {
    fprintf(stream, "\t.int %s\n", str);
}

zero_direct(stream, n) {
    fprintf(stream, "\t.zero %d\n", n);
}

