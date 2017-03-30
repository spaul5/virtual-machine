/*
The MIT License (MIT)

Copyright (c) 2015 Terence Parr, Hanzhou Shi, Shuai Yuan, Yuanyuan Zhang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vm.h"
#include "loader.h"

VM_INSTRUCTION vm_instructions[] = {
	{"HALT",  HALT,  {}, 0},

	{"IADD",  IADD,  {}, 2},
	{"ISUB",  ISUB,  {}, 2},
	{"IMUL",  IMUL,  {}, 2},
	{"IDIV",  IDIV,  {}, 2},
	{"SADD",  SADD,  {}, 2},

	{"OR",    OR,    {}, 2},
	{"AND",   AND,   {}, 2},
	{"INEG",  INEG,  {}, 1},
	{"NOT",   NOT,   {}, 1},

	{"I2S",   I2S,   {}, 1},

	{"IEQ",   IEQ,   {}, 2},
	{"INEQ",  INEQ,  {}, 2},
	{"ILT",   ILT,   {}, 2},
	{"ILE",   ILE,   {}, 2},
	{"IGT",   IGT,   {}, 2},
	{"IGE",   IGE,   {}, 2},
	{"SEQ",   SEQ,   {}, 2},
	{"SNEQ",  SNEQ,  {}, 2},
	{"SGT",   SGT,   {}, 2},
	{"SGE",   SGE,   {}, 2},
	{"SLT",	  SLT,	 {}, 2},
	{"SLE",	  SLE,	 {}, 2},

	{"BR",	  BR,	 {4}, 0}, // branch to absolute address
	{"BRF",	  BRF,	 {4}, 0},

	{"ICONST",	ICONST,	   	{4}, 0},
	{"SCONST",	SCONST,	   	{2}, 0},

	{"LOAD",	LOAD,	    {2}, 0},
	{"STORE",	STORE,	    {2}, 1},
	{"SINDEX", 	SINDEX,		{},  2},

	{"POP",		POP,		{},  0},
	{"CALL",	CALL,	    {4,2}, 0}, // num stack operands is actually a variable and 2nd operand
	{"LOCALS",	LOCALS,	    {2}, 0},
	{"RET",		RET,		{},  1},

	{"PRINT",	PRINT,	   	{},  1},
	{"SLEN",	SLEN,	   	{},  1},
	{"SFREE",	SFREE,	   	{2}, 0} // free a str in a local
};

static void vm_print_instr(VM *vm, addr32 ip);
static void vm_print_stack(VM *vm);
static char *vm_print_element(char *buffer, element el);
static inline int32_t int32(const byte *data, addr32 ip);
static inline int16_t int16(const byte *data, addr32 ip);
static void vm_trace_print_element(VM *vm, element el);

VM *vm_alloc() {
	VM *vm = calloc(1, sizeof(VM));
	vm->trace = (char *) malloc((999)*sizeof(char));
	vm->output = (char *) malloc((999)*sizeof(char));
	return vm;
}

void vm_init(VM *vm, byte *code, int code_size)
{
	vm->code = code;
	vm->code_size = code_size;
	vm->sp = -1; // grow upwards, stack[sp] is top of stack and valid
	vm->callsp = -1;
}

void vm_free(VM *vm) {
	free(vm->strings);
	free(vm->func_names);
	free(vm->trace);
	free(vm->output);
	free(vm->code);
	free(vm);
}

// You don't need to use/create these functions but I used them
// to help me when there are bugs in my code.
static void inline validate_stack_address(VM *vm, int a) { }
static void inline validate_stack(VM *vm, byte opcode, int sp) { }

void vm_exec(VM *vm, bool trace_to_stderr) {


//	bool trace = true; // always store trace in vm->trace
//	if (trace) {
//		WRITE_BACK_REGISTERS(vm);
//		vm_print_stack(vm);
//		if ( trace_to_stderr ) fprintf(stderr, "%s", cur_trace);
//	}

	addr32 ip = 0;         // instruction pointer register    // call stack pointer register

	int x = 0;
	int y = 0;
	String *s = NULL;
	String *t = NULL;
	String *output = NULL;
	element e;
	Activation_Record ar;
	int addr = 0;
	int offset = 0;


	// main function
	ar.name = vm->func_names[0];
	ar.nargs = 0;
	ar.nlocals = 0;
	vm->call_stack[++vm->callsp] = ar;


//	vm->ip
	byte opcode = vm->code[ip];

	while (ip < vm->code_size) {
		printf("ip: %d\n", ip);
		printf("opcode: %d\n", opcode);
		if (trace_to_stderr) vm_print_instr(vm, ip);
		ip++;

		switch(opcode) {
			case HALT:
				break;
			case IADD:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = INT, .i = x+y};
				vm->stack[++vm->sp] = e;
				break;
			case ISUB:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = INT, .i = x-y};
				vm->stack[++vm->sp] = e;
				break;
			case IMUL:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = INT, .i = x*y};
				vm->stack[++vm->sp] = e;
				break;
			case IDIV:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = INT, .i = x/y};
				vm->stack[++vm->sp] = e;
				break;
			case SADD:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = STRING, .s = String_add(s, t)};
				vm->stack[++vm->sp] = e;
				break;
			case OR:
				y = vm->stack[vm->sp--].i; //fix. might have to convert to bit first
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = INT, .i = (x|y)}; // then back to int
				vm->stack[++vm->sp] = e;
				break;
			case AND:

				break;
			case INEG:

				break;
			case NOT:

				break;
			case I2S:
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = STRING, .s = String_from_int(x)};
				vm->stack[++vm->sp] = e;
				break;
			case IEQ:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x==y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case INEQ:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x!=y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case ILT:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x<y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case ILE:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x<=y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case IGT:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x>y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case IGE:
				y = vm->stack[vm->sp--].i;
				x = vm->stack[vm->sp--].i;
				e = (element) {.type = BOOLEAN, .b = x>=y ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SEQ:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_eq(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SNEQ:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_neq(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SGT:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_gt(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SGE:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_ge(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SLT:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_lt(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;
			case SLE:
				t = vm->stack[vm->sp--].s;	// ->s ??
				s = vm->stack[vm->sp--].s;
				e = (element) {.type = BOOLEAN, .b = String_le(s, t) ? true : false };
				vm->stack[++vm->sp] = e;
				break;

			case BR:
				ip = int16(vm->code, ip);
//				ip = ip + 4;
				break;
			case BRF:
				if (vm->stack[vm->sp--].b) {
					ip = int16(vm->code, ip);
				}
				else {
					ip = ip + 4;
				}
				break;
			case ICONST:
				printf("iconst\n");
				e = (element) {.type = INT, .i = int16(vm->code, ip)};
				ip = ip + 4;
				vm->stack[++vm->sp] = e;
				printf("e.i %d\n", e.i);
				break;
			case SCONST:
				e = (element) {.type = STRING, .s = String_dup(vm->strings[int16(vm->code, ip)])};
				vm->stack[++vm->sp] = e;
				ip = ip + 2;
				break;
			case LOAD:
				printf("load\n");
//				e = ar.locals[vm->code[ip++]];
				e = ar.locals[int16(vm->code, ip)];
				printf("%d\n", e.i);
				vm->stack[++vm->sp] = e;
				ip = ip + 2;
				break;
			case STORE:
//				ar.locals[vm->code[ip++]] = vm->stack[sp--];
				ar = vm->call_stack[vm->callsp--];
				ar.locals[int16(vm->code, ip)] = vm->stack[vm->sp--];
				vm->call_stack[++vm->callsp] = ar;
				ip = ip + 2;
				break;
			case SINDEX:
				x = vm->stack[vm->sp--].i;
				s = vm->stack[vm->sp--].s;
//				e = (element) {.type = STRING, .s = s[x]};
//				e = (element) {.type = STRING, .s = s[x]};
				vm->stack[++vm->sp] = e;
				break;
			case POP:
				printf("pop\n");
				vm->sp--;
				break;
			case CALL:
				printf("call\n");
				ar.nargs = int16(vm->code, ip+4);
				printf("nargs %d\n", ar.nargs);
				ar.retaddr = int16(vm->code, ip+7); // what?
//				vm->call_stack[++vm->callsp] = ar;
				for (int i=int16(vm->code, ip+4)-1; i>=0; i--) {
					ar.locals[i] = vm->stack[vm->sp--];
					printf("%d\n", ar.locals[i].i);
				}
				ar.name = vm->func_names[int16(vm->code, ip)];
				ip = int16(vm->code, ip);
				//ip = ip + 7;

				vm->call_stack[++vm->callsp] = ar;
				break;
			case LOCALS:
				ar = vm->call_stack[vm->callsp--];
				ar.nlocals = int16(vm->code, ip);
				vm->call_stack[++vm->callsp] = ar;
				ip = ip + 2;
				break;
			case RET:
				printf("ret\n");
				x = vm->stack[vm->sp--].i;
				ar = vm->call_stack[vm->callsp--];
				e = (element) {.type = INT, .i = x};
				vm->stack[++vm->sp] = e;
				ip = ar.retaddr;
//				ip = vm->call_stack[vm->callsp--].retaddr;
				break;
			case PRINT:
				printf("print\n");
				e = vm->stack[vm->sp--];
				switch (e.type) {
					case INT:
						x = e.i;
						printf("%d\n", e.i);
//						String *str = String_from_int(x);
//						str = String_add(str, String_from_char('\n'));
						output = String_add(output, String_from_int(x));
						output = String_add(output, String_from_char('\n'));
//						output = String_add(output, str);
						break;
					case STRING:
						printf("%d\n", e.s);
						output = String_add(output, e.s);
						break;
					case BOOLEAN:
						if (e.b) {
							output = String_add(output, String_new("true\n"));
						}
						else {
							output = String_add(output, String_new("false\n"));
						}
						break;
				}
				break;
			case SLEN:
				e = (element) {.type = INT, .i = String_len(vm->stack[vm->sp--].s)};
				vm->stack[++vm->sp] = e;
				break;
			case SFREE:
				ar = vm->call_stack[vm->callsp--];
				e = ar.locals[int16(vm->code, ip)];
				free(e.s);
				e.type = INVALID;
				e.s = NULL;
				ar.locals[int16(vm->code, ip)] = e;
				vm->call_stack[++vm->callsp] = ar;
				ip = ip + 2;
				break;
			default:
				printf("invalid opcode: %d at ip=%d\n", opcode, (ip - 1));
				exit(1);
		}

		if (trace_to_stderr) vm_print_stack(vm);
		opcode = vm->code[ip];


	}
//	vm->call_stack[vm->callsp--];

//	strcpy(vm->output, output.c_str());
	printf("my output: %s\n", output->str);
	e = (element) {.type = STRING, .s = output};
	vm->output = vm_print_element(vm->output, e);

}

/* return a 32-bit integer at data[ip] */
static inline int32_t int32(const byte *data, addr32 ip)
{
	return *((int32_t *)&data[ip]);
}

/* return a 16-bit integer at data[ip] */
static inline int16_t int16(const byte *data, addr32 ip)
{
	return *((int16_t *)&data[ip]); // could be negative value
}

void vm_print_instr_opnd0(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	print(vm->trace, "%04d:  %-25s", ip, inst->name);
}

void vm_print_instr_opnd1(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	int sz = inst->opnd_sizes[0];
	switch (sz) {
		case 2:
			print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int16(vm->code, ip + 1));
			break;
		case 4:
			print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int32(vm->code, ip + 1));
			break;
		default:
			break;
	}
}

/* currently only a CALL instr */
void vm_print_instr_opnd2(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	char buf[100];
	sprintf(buf, "%d, %d", int32(vm->code, ip + 1), int16(vm->code, ip + 5));
	print(vm->trace, "%04d:  %-15s%-10s", ip, inst->name, buf);
}

static void vm_print_instr(VM *vm, addr32 ip)
{
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	if ( inst->opnd_sizes[1]>0 ) {
		vm_print_instr_opnd2(vm, ip);
	}
	else if ( inst->opnd_sizes[0]>0 ) {
		vm_print_instr_opnd1(vm, ip);
	}
	else {
		vm_print_instr_opnd0(vm, ip);
	}
}

static void vm_print_stack(VM *vm) {
	// stack grows upwards; stack[sp] is top of stack
	print(vm->trace, "calls=[");
	for (int i = 0; i <= vm->callsp; i++) {
		Activation_Record *frame = &vm->call_stack[i];
		print(vm->trace, " %s=[", frame->name);
		for (int j = 0; j < frame->nlocals+frame->nargs; ++j) {
			print(vm->trace, " ");
			vm_trace_print_element(vm, frame->locals[j]);
		}
		print(vm->trace, " ]");
	}
	print(vm->trace, " ]  ");
	print(vm->trace, "stack=[");
	for (int i = 0; i <= vm->sp; i++) {
		print(vm->trace, " ");
		vm_trace_print_element(vm, vm->stack[i]);
	}
	print(vm->trace, " ] sp=%d\n", vm->sp);
}

void vm_trace_print_element(VM *vm, element el) {
	if (el.type == STRING) {
		print(vm->trace, "\"");
		vm_print_element(vm->trace, el);
		print(vm->trace, "\"");
	}
	else {
		vm_print_element(vm->trace, el);
	}
}

char *vm_print_element(char *buffer, element el) {
	switch ( el.type ) {
		case INT :
			return print(buffer, "%d", el.i);
		case BOOLEAN :
			return print(buffer, "%s", el.b ? "true" : "false");
		case STRING :
			return print(buffer, "%s", el.s->str);
		default:
			return print(buffer, "%s", "?");
	}
}

char *print(char *buffer, char *fmt, ...) {
	va_list args;
	char buf[1000];

	size_t n = strlen(buffer);
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	strcat(buffer, buf);
	va_end(args);
	return &buffer[n];
}
