#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vm.h>

#include "vm.h"
#include "c_unit.h"
#include "loader.h"

static VM *run(char *code, bool trace);

// globals so we can free them upon failure (which bails out of test functions)

static char *diff = NULL;
static VM *vm;

static void setup() {
	diff = NULL;
	vm = NULL;
}

static void teardown() {
	if ( diff!=NULL ) {
		free(diff);
	}
	if ( vm!=NULL ) {
		vm_free(vm);
	}
}

/* int main() { foo(); }
 * int foo() { print 1234; return 0; }
 */
void call_hello() {
	char *code =
		"1 strings\n"
		"   0: 5/hello\n"
		"2 functions maxaddr=9\n"
		"	0: 4/main\n"
		"	9: 3/foo\n"
		"7 instr, 21 bytes\n"
		// main:
		// foo()
		"	CALL 9, 0\n"
		"	POP\n"	// drop return value
		"	HALT\n"
		// foo:
		// print 1234
		"	ICONST 1234\n"
		"	PRINT\n"
		// return 0
		"	ICONST 0\n" // return 0;
		"   RET\n";
	char *expected_output = "1234\n";
	char *expected_trace =
		"0000:  CALL           9, 0      calls=[ main=[ ] foo=[ ] ]  stack=[ ] sp=-1\n"
		"0009:  ICONST         1234      calls=[ main=[ ] foo=[ ] ]  stack=[ 1234 ] sp=0\n"
		"0014:  PRINT                    calls=[ main=[ ] foo=[ ] ]  stack=[ ] sp=-1\n"
		"0015:  ICONST         0         calls=[ main=[ ] foo=[ ] ]  stack=[ 0 ] sp=0\n"
		"0020:  RET                      calls=[ main=[ ] ]  stack=[ 0 ] sp=0\n"
		"0007:  POP                      calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0008:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";

	vm = run(code, false);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void print_arg() {
	char *code =
		"0 strings\n"
		"2 functions maxaddr=14\n"
		"	0: 4/main\n"
		"	14: 3/foo\n"
		"8 instr, 24 bytes\n"
		// main:
		// foo(1234)
		"	ICONST 1234\n"			// 0
		"	CALL 14, 1\n"			// 5
		"	POP\n"					// 12
		"	HALT\n"					// 13
		// foo(int x):
		// print x
		"	LOAD 0\n"				// 14
		"	PRINT\n"
		// return 0
		"	ICONST 0\n" // return 0;
		"   RET\n";
	char *expected_output = "1234\n";
	char *expected_trace =
		"0000:  ICONST         1234      calls=[ main=[ ] ]  stack=[ 1234 ] sp=0\n"
		"0005:  CALL           14, 1     calls=[ main=[ ] foo=[ 1234 ] ]  stack=[ ] sp=-1\n"
		"0014:  LOAD           0         calls=[ main=[ ] foo=[ 1234 ] ]  stack=[ 1234 ] sp=0\n"
		"0017:  PRINT                    calls=[ main=[ ] foo=[ 1234 ] ]  stack=[ ] sp=-1\n"
		"0018:  ICONST         0         calls=[ main=[ ] foo=[ 1234 ] ]  stack=[ 0 ] sp=0\n"
		"0023:  RET                      calls=[ main=[ ] ]  stack=[ 0 ] sp=0\n"
		"0012:  POP                      calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0013:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";

	vm = run(code, false);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void print_args() {
	char *code =
		"0 strings\n"
		"2 functions maxaddr=19\n"
		"	0: 4/main\n"
		"	19: 3/foo\n"
		"11 instr, 33 bytes\n"
		// main:
		// foo(99,100)
		"	ICONST 99\n"			// 0
		"	ICONST 100\n"			// 5
		"	CALL 19, 2\n"			// 10
		"	PRINT\n"				// 17
		"	HALT\n"					// 18
		// foo(x:int, y:int):
		// print x
		"	LOAD 0\n"				// 19
		"	PRINT\n"
		"	LOAD 1\n"
		"	PRINT\n"
		// return 101
		"	ICONST 101\n"
		"   RET\n";
	char *expected_output = "99\n100\n101\n";
	char *expected_trace =
		"0000:  ICONST         99        calls=[ main=[ ] ]  stack=[ 99 ] sp=0\n"
		"0005:  ICONST         100       calls=[ main=[ ] ]  stack=[ 99 100 ] sp=1\n"
		"0010:  CALL           19, 2     calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ ] sp=-1\n"
		"0019:  LOAD           0         calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ 99 ] sp=0\n"
		"0022:  PRINT                    calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ ] sp=-1\n"
		"0023:  LOAD           1         calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ 100 ] sp=0\n"
		"0026:  PRINT                    calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ ] sp=-1\n"
		"0027:  ICONST         101       calls=[ main=[ ] foo=[ 99 100 ] ]  stack=[ 101 ] sp=0\n"
		"0032:  RET                      calls=[ main=[ ] ]  stack=[ 101 ] sp=0\n"
		"0017:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0018:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";

	vm = run(code, false);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void arg_and_local() {
	char *code =
		"0 strings\n"
		"2 functions maxaddr=14\n"
		"	0: 4/main\n"
		"	14: 3/foo\n"
		"11 instr, 33 bytes\n"
		// main:
		// foo(1234)
		"	ICONST 1234\n"			// 0
		"	CALL 14, 1\n"			// 5
		"	POP\n"					// 12
		"	HALT\n"					// 13
		// foo(x:int):
		// var y:int;
		"	LOCALS 1\n"				// 14
		// y = x
		"	LOAD 0\n"				// 17
		"	STORE 1\n"				// 20
		// print y
		"	LOAD 1\n"
		"	PRINT\n"
		// return 0
		"	ICONST 0\n" // return 0;
		"   RET\n";
	char *expected_output = "1234\n";
	char *expected_trace =
		"0000:  ICONST         1234      calls=[ main=[ ] ]  stack=[ 1234 ] sp=0\n"
		"0005:  CALL           14, 1     calls=[ main=[ ] foo=[ 1234 ] ]  stack=[ ] sp=-1\n"
		"0014:  LOCALS         1         calls=[ main=[ ] foo=[ 1234 ? ] ]  stack=[ ] sp=-1\n"
		"0017:  LOAD           0         calls=[ main=[ ] foo=[ 1234 ? ] ]  stack=[ 1234 ] sp=0\n"
		"0020:  STORE          1         calls=[ main=[ ] foo=[ 1234 1234 ] ]  stack=[ ] sp=-1\n"
		"0023:  LOAD           1         calls=[ main=[ ] foo=[ 1234 1234 ] ]  stack=[ 1234 ] sp=0\n"
		"0026:  PRINT                    calls=[ main=[ ] foo=[ 1234 1234 ] ]  stack=[ ] sp=-1\n"
		"0027:  ICONST         0         calls=[ main=[ ] foo=[ 1234 1234 ] ]  stack=[ 0 ] sp=0\n"
		"0032:  RET                      calls=[ main=[ ] ]  stack=[ 0 ] sp=0\n"
		"0012:  POP                      calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0013:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";

	vm = run(code, false);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void fib() {
	char *code =
		"0 strings\n"
		"2 functions maxaddr=62\n"
		"    0: 3/fib\n"
		"    62: 4/main\n"
		"27 instr, 89 bytes\n"
		// fib(x:int) : int
		// if x==0 or x==1
		"    LOAD 0\n"			// 0
		"    ICONST 0\n"		// 3
		"    IEQ\n"				// 8
		"    LOAD 0\n"			// 9
		"    ICONST 1\n"		// 12
		"    IEQ\n"				// 17
		"    OR\n"				// 18
		"    BRF 28\n"			// 19
		// return 0
		"    LOAD 0\n"			// 24
		"    RET\n"				// 27
		// else return fib(x-1)+fib(x-2)
		"    LOAD 0\n"			// 28
		"    ICONST 1\n"
		"    ISUB\n"
		"    CALL 0, 1\n"
		"    LOAD 0\n"
		"    ICONST 2\n"
		"    ISUB\n"
		"    CALL 0, 1\n"
		"    IADD\n"
		"    RET\n"
		// main()
		// print fib(1)
		"    ICONST 1\n"		// 62
		"    CALL 0, 1\n"		// 67
		"    PRINT\n"			// 74
		// print fib(5)
		"    ICONST 3\n"		// 75
		"    CALL 0, 1\n"		// 80
		"    PRINT\n"			// 87
		"    HALT\n";			// 88
	char *expected_output = "1\n2\n";
	char *expected_trace =
		"0062:  ICONST         1         calls=[ main=[ ] ]  stack=[ 1 ] sp=0\n"
		"0067:  CALL           0, 1      calls=[ main=[ ] fib=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 1 ] ]  stack=[ 1 0 ] sp=1\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 1 ] ]  stack=[ false ] sp=0\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 1 ] ]  stack=[ false 1 ] sp=1\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 1 ] ]  stack=[ false 1 1 ] sp=2\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 1 ] ]  stack=[ false true ] sp=1\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 1 ] ]  stack=[ true ] sp=0\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0024:  LOAD           0         calls=[ main=[ ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0027:  RET                      calls=[ main=[ ] ]  stack=[ 1 ] sp=0\n"
		"0074:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0075:  ICONST         3         calls=[ main=[ ] ]  stack=[ 3 ] sp=0\n"
		"0080:  CALL           0, 1      calls=[ main=[ ] fib=[ 3 ] ]  stack=[ ] sp=-1\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 3 ] sp=0\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 3 0 ] sp=1\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 3 ] ]  stack=[ false ] sp=0\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ false 3 ] sp=1\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ false 3 1 ] sp=2\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 3 ] ]  stack=[ false false ] sp=1\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 3 ] ]  stack=[ false ] sp=0\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 3 ] ]  stack=[ ] sp=-1\n"
		"0028:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 3 ] sp=0\n"
		"0031:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 3 1 ] sp=1\n"
		"0036:  ISUB                     calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 2 ] sp=0\n"
		"0037:  CALL           0, 1      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ ] sp=-1\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 2 ] sp=0\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 2 0 ] sp=1\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ false ] sp=0\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ false 2 ] sp=1\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ false 2 1 ] sp=2\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ false false ] sp=1\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ false ] sp=0\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ ] sp=-1\n"
		"0028:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 2 ] sp=0\n"
		"0031:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 2 1 ] sp=1\n"
		"0036:  ISUB                     calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 ] sp=0\n"
		"0037:  CALL           0, 1      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ 1 0 ] sp=1\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ false ] sp=0\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ false 1 ] sp=1\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ false 1 1 ] sp=2\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ false true ] sp=1\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ true ] sp=0\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0024:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0027:  RET                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 ] sp=0\n"
		"0044:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 2 ] sp=1\n"
		"0047:  ICONST         2         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 2 2 ] sp=2\n"
		"0052:  ISUB                     calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 0 ] sp=1\n"
		"0053:  CALL           0, 1      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 ] sp=0\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 0 ] sp=1\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 0 0 ] sp=2\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 true ] sp=1\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 true 0 ] sp=2\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 true 0 1 ] sp=3\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 true false ] sp=2\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 true ] sp=1\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 ] sp=0\n"
		"0024:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] fib=[ 0 ] ]  stack=[ 1 0 ] sp=1\n"
		"0027:  RET                      calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 0 ] sp=1\n"
		"0060:  IADD                     calls=[ main=[ ] fib=[ 3 ] fib=[ 2 ] ]  stack=[ 1 ] sp=0\n"
		"0061:  RET                      calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 1 ] sp=0\n"
		"0044:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 1 3 ] sp=1\n"
		"0047:  ICONST         2         calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 1 3 2 ] sp=2\n"
		"0052:  ISUB                     calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 1 1 ] sp=1\n"
		"0053:  CALL           0, 1      calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0000:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 1 ] sp=1\n"
		"0003:  ICONST         0         calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 1 0 ] sp=2\n"
		"0008:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 false ] sp=1\n"
		"0009:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 false 1 ] sp=2\n"
		"0012:  ICONST         1         calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 false 1 1 ] sp=3\n"
		"0017:  IEQ                      calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 false true ] sp=2\n"
		"0018:  OR                       calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 true ] sp=1\n"
		"0019:  BRF            28        calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0024:  LOAD           0         calls=[ main=[ ] fib=[ 3 ] fib=[ 1 ] ]  stack=[ 1 1 ] sp=1\n"
		"0027:  RET                      calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 1 1 ] sp=1\n"
		"0060:  IADD                     calls=[ main=[ ] fib=[ 3 ] ]  stack=[ 2 ] sp=0\n"
		"0061:  RET                      calls=[ main=[ ] ]  stack=[ 2 ] sp=0\n"
		"0087:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0088:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";

	vm = run(code, false);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

int main(int argc, char *argv[]) {
	c_unit_setup = setup;
	c_unit_teardown = teardown;

//	test(call_hello);
//	test(print_arg);
	test(print_args);
//	test(arg_and_local);
//	test(fib);

	return c_unit_fails;
}

// S U P P O R T

static VM *run(char *code, bool trace) {
	save_string_in_file("t.bytecode", code);
	char fname[400];
	strcpy(fname, get_temp_dir());
	strcat(fname, "/t.bytecode");
	FILE *f = fopen(fname, "r");
	VM *vm = vm_load(f);
	fclose(f);
	vm_exec(vm,trace);
	return vm;
}