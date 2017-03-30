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

/*
 * print 1234
 */
void hello() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"3 instr, 7 bytes\n"
		"	ICONST 1234\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "1234\n";
	char *expected_trace =
		"0000:  ICONST         1234      calls=[ main=[ ] ]  stack=[ 1234 ] sp=0\n"
		"0005:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
		"0006:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";
	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void locals() {
	char *code =
	"0 strings\n"
	"1 functions maxaddr=0\n"
	"	0: 4/main\n"
	"10 instr, 28 bytes\n"
	"	LOCALS 2\n"
	"	ICONST 99\n"
	"	STORE 0\n"
	"	LOAD 0\n"
	"	PRINT\n"
	"	ICONST 1234\n"
	"	STORE 1\n"
	"	LOAD 1\n"
	"	PRINT\n"
	"	HALT\n";
	char *expected_output = "99\n1234\n";
	char *expected_trace =
	"0000:  LOCALS         2         calls=[ main=[ ? ? ] ]  stack=[ ] sp=-1\n"
	"0003:  ICONST         99        calls=[ main=[ ? ? ] ]  stack=[ 99 ] sp=0\n"
	"0008:  STORE          0         calls=[ main=[ 99 ? ] ]  stack=[ ] sp=-1\n"
	"0011:  LOAD           0         calls=[ main=[ 99 ? ] ]  stack=[ 99 ] sp=0\n"
	"0014:  PRINT                    calls=[ main=[ 99 ? ] ]  stack=[ ] sp=-1\n"
	"0015:  ICONST         1234      calls=[ main=[ 99 ? ] ]  stack=[ 1234 ] sp=0\n"
	"0020:  STORE          1         calls=[ main=[ 99 1234 ] ]  stack=[ ] sp=-1\n"
	"0023:  LOAD           1         calls=[ main=[ 99 1234 ] ]  stack=[ 1234 ] sp=0\n"
	"0026:  PRINT                    calls=[ main=[ 99 1234 ] ]  stack=[ ] sp=-1\n"
	"0027:  HALT                     calls=[ main=[ 99 1234 ] ]  stack=[ ] sp=-1\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void sfree() {
	char *code =
		"1 strings\n"
		"   0: 5/hello\n"	// vm_free() frees this during teardown()
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 15 bytes\n"
		"	LOCALS 1\n"
		"	SCONST 0\n"		// makes copy of String managed by vm so we must manually free it
		"	STORE 0\n"
		"	SFREE 0\n"
		"	HALT\n";
	char *expected_output = "";
	char *expected_trace =
		"0000:  LOCALS         1         calls=[ main=[ ? ] ]  stack=[ ] sp=-1\n"
		"0003:  SCONST         0         calls=[ main=[ ? ] ]  stack=[ \"hello\" ] sp=0\n"
		"0006:  STORE          0         calls=[ main=[ \"hello\" ] ]  stack=[ ] sp=-1\n"
		"0009:  SFREE          0         calls=[ main=[ ? ] ]  stack=[ ] sp=-1\n"
		"0012:  HALT                     calls=[ main=[ ? ] ]  stack=[ ] sp=-1\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void slen() {
	char *code =
		"1 strings\n"
		"   0: 5/hello\n"	// vm_free() frees this during teardown()
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"8 instr, 20 bytes\n"
		"	LOCALS 1\n"
		"	SCONST 0\n"		// makes copy of String managed by vm so we must manually free it
		"	STORE 0\n"
		"	LOAD 0\n"
		"	SLEN\n"
		"	PRINT\n"
		"	SFREE 0\n"
		"	HALT\n";
	char *expected_output = "5\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void iadd() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 13 bytes\n"
		"	ICONST 3\n"
		"	ICONST 4\n"
		"	IADD\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "7\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void isub_pos() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 13 bytes\n"
		"	ICONST 10\n"
		"	ICONST 4\n"
		"	ISUB\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "6\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void isub_neg() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 13 bytes\n"
		"	ICONST 3\n"
		"	ICONST 4\n"
		"	ISUB\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "-1\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void imul() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 13 bytes\n"
		"	ICONST 10\n"
		"	ICONST 4\n"
		"	IMUL\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "40\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void idiv() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"5 instr, 13 bytes\n"
		"	ICONST 10\n"
		"	ICONST 4\n"
		"	IDIV\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output = "2\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void ieq() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"17 instr, 49 bytes\n"
		"	ICONST 4\n"
		"	ICONST 4\n"
		"	IEQ\n"
		"	PRINT\n"
		"	ICONST 5\n"
		"	ICONST 4\n"
		"	IEQ\n"
		"	PRINT\n"
		"	ICONST 4\n"
		"	ICONST 4\n"
		"	INEQ\n"
		"	PRINT\n"
		"	ICONST 5\n"
		"	ICONST 4\n"
		"	INEQ\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output =
		"true\n"
		"false\n"
		"false\n"
		"true\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void icmp() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"29 instr, 85 bytes\n"
		"	ICONST 3\n"
		"	ICONST 4\n"
		"	ILT\n"
		"	PRINT\n"
		"	ICONST 8\n"
		"	ICONST 4\n"
		"	ILT\n"
		"	PRINT\n"
		"	ICONST 8\n"
		"	ICONST 4\n"
		"	IGT\n"
		"	PRINT\n"
		"	ICONST 3\n"
		"	ICONST 4\n"
		"	IGT\n"
		"	PRINT\n"
		"	ICONST 3\n"
		"	ICONST 4\n"
		"	ILE\n"
		"	PRINT\n"
		"	ICONST 6\n"
		"	ICONST 4\n"
		"	ILE\n"
		"	PRINT\n"
		"	ICONST 4\n"
		"	ICONST 4\n"
		"	IGE\n"
		"	PRINT\n"
		"	HALT\n";
	char *expected_output =
		"true\n"
		"false\n"
		"true\n"
		"false\n"
		"true\n"
		"false\n"
		"true\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

/* var s= "hello"+"bye";
 * print s;
 * free(s);
 */
void sadd() {
	char *code =
	"2 strings\n"
	"   0: 5/hello\n"
	"   1: 3/bye\n"
	"1 functions maxaddr=0\n"
	"	0: 4/main\n"
	"15 instr, 39 bytes\n"
	"	LOCALS 3\n"

	"	SCONST 0\n"
	"	STORE 0\n"
	"	SCONST 1\n"
	"	STORE 1\n"

	"	LOAD 0\n"
	"	LOAD 1\n"
	"	SADD\n"
	"	STORE 2\n"

	"	LOAD 2\n"
	"	PRINT\n"

	"	SFREE 0\n"
	"	SFREE 1\n"
	"	SFREE 2\n"
	"	HALT\n";
	char *expected_output = "hellobye\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void i2s() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"8 instr, 25 bytes\n"
		"	LOCALS 1\n"
		"	ICONST 9324\n"
		"	I2S\n"			// creates string we must free; means must store into local for SFREE
		"	STORE 0\n"
		"	LOAD 0\n"
		"	PRINT\n"
		"	SFREE 0\n"
		"	HALT\n";
	char *expected_output = "9324\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void seq() {
	char *code =
	"2 strings\n"
	"   0: 3/foo\n"
	"   1: 3/bar\n"
	"1 functions maxaddr=0\n"
	"	0: 4/main\n"
	"24 instr, 54 bytes\n"
	"	LOCALS 2\n"

	"	SCONST 0\n"
	"	STORE 0\n"
	"	SCONST 1\n"
	"	STORE 1\n"

	"	LOAD 0\n"
	"	LOAD 0\n"
	"	SEQ\n"
	"	PRINT\n"

	"	LOAD 0\n"
	"	LOAD 1\n"
	"	SEQ\n"
	"	PRINT\n"

	"	LOAD 0\n"
	"	LOAD 0\n"
	"	SNEQ\n"
	"	PRINT\n"

	"	LOAD 0\n"
	"	LOAD 1\n"
	"	SNEQ\n"
	"	PRINT\n"

	"	SFREE 0\n"
	"	SFREE 1\n"
	"	HALT\n";
	char *expected_output =
	"true\n"
	"false\n"
	"false\n"
	"true\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

void scmp() {
	char *code =
		"4 strings\n"
		"   0: 3/bat\n"
		"   1: 3/cat\n"
		"   2: 3/dog\n"
		"   3: 6/doggie\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"62 instr, 184 bytes\n"
		"	LOCALS 4\n"

		"	SCONST 0\n"
		"	STORE 0\n"
		"	SCONST 1\n"
		"	STORE 1\n"
		"	SCONST 2\n"
		"	STORE 2\n"
		"	SCONST 3\n"
		"	STORE 3\n"

		"	LOAD 0\n"
		"	LOAD 1\n"
		"	SLT\n"
		"	PRINT\n"

		"	LOAD 0\n"
		"	LOAD 0\n"
		"	SLT\n"
		"	PRINT\n"

		"	LOAD 1\n"
		"	LOAD 0\n"
		"	SGT\n"
		"	PRINT\n"

		"	LOAD 0\n"
		"	LOAD 1\n"
		"	SGT\n"
		"	PRINT\n"

		"	LOAD 0\n"
		"	LOAD 0\n"
		"	SLE\n"
		"	PRINT\n"

		"	LOAD 2\n"
		"	LOAD 1\n"
		"	SLE\n"
		"	PRINT\n"

		"	LOAD 0\n"
		"	LOAD 1\n"
		"	SLE\n"
		"	PRINT\n"

		"	LOAD 2\n"
		"	LOAD 1\n"
		"	SGE\n"
		"	PRINT\n"

		"	LOAD 1\n"
		"	LOAD 1\n"
		"	SGE\n"
		"	PRINT\n"

		"	LOAD 2\n"		// dog >= doggie  false
		"	LOAD 3\n"
		"	SGE\n"
		"	PRINT\n"

		"	LOAD 2\n"		// dog < doggie	  true
		"	LOAD 3\n"
		"	SLT\n"
		"	PRINT\n"

		"	LOAD 3\n"
		"	LOAD 2\n"
		"	SLE\n"
		"	PRINT\n"

		"	SFREE 0\n"
		"	SFREE 1\n"
		"	SFREE 2\n"
		"	SFREE 3\n"
		"	HALT\n";
	char *expected_output =
		"true\n"
		"false\n"
		"true\n"
		"false\n"
		"true\n"
		"false\n"
		"true\n"
		"true\n"
		"true\n"
		"false\n"
		"true\n"
		"false\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);
}

/* print "hello"[1]
 * print "hello"[5]
 */
void sindex() { // indexing is from 1 NOT 0
	char *code =
	"1 strings\n"
	"   0: 5/hello\n"
	"1 functions maxaddr=7\n"
	"	0: 4/main\n"
	"19 instr, 51 bytes\n"
	"	LOCALS 2\n"
	"	SCONST 0\n"
	"	STORE 0\n"

	"	LOAD 0\n"
	"	ICONST 1\n"
	"	SINDEX\n"
	"	STORE 1\n"
	"	LOAD 1\n"
	"	PRINT\n"
	"	SFREE 1\n"

	"	LOAD 0\n"
	"	ICONST 5\n"
	"	SINDEX\n"
	"	STORE 1\n"
	"	LOAD 1\n"
	"	PRINT\n"
	"	SFREE 1\n"

	"	SFREE 0\n"	// ugh; manual GC sucks
	"	HALT\n";
	char *expected_output = "h\no\n";
	char *expected_trace =
		"0000:  LOCALS         2         calls=[ main=[ ? ? ] ]  stack=[ ] sp=-1\n"
		"0003:  SCONST         0         calls=[ main=[ ? ? ] ]  stack=[ \"hello\" ] sp=0\n"
		"0006:  STORE          0         calls=[ main=[ \"hello\" ? ] ]  stack=[ ] sp=-1\n"
		"0009:  LOAD           0         calls=[ main=[ \"hello\" ? ] ]  stack=[ \"hello\" ] sp=0\n"
		"0012:  ICONST         1         calls=[ main=[ \"hello\" ? ] ]  stack=[ \"hello\" 1 ] sp=1\n"
		"0017:  SINDEX                   calls=[ main=[ \"hello\" ? ] ]  stack=[ \"h\" ] sp=0\n"
		"0018:  STORE          1         calls=[ main=[ \"hello\" \"h\" ] ]  stack=[ ] sp=-1\n"
		"0021:  LOAD           1         calls=[ main=[ \"hello\" \"h\" ] ]  stack=[ \"h\" ] sp=0\n"
		"0024:  PRINT                    calls=[ main=[ \"hello\" \"h\" ] ]  stack=[ ] sp=-1\n"
		"0025:  SFREE          1         calls=[ main=[ \"hello\" ? ] ]  stack=[ ] sp=-1\n"
		"0028:  LOAD           0         calls=[ main=[ \"hello\" ? ] ]  stack=[ \"hello\" ] sp=0\n"
		"0031:  ICONST         5         calls=[ main=[ \"hello\" ? ] ]  stack=[ \"hello\" 5 ] sp=1\n"
		"0036:  SINDEX                   calls=[ main=[ \"hello\" ? ] ]  stack=[ \"o\" ] sp=0\n"
		"0037:  STORE          1         calls=[ main=[ \"hello\" \"o\" ] ]  stack=[ ] sp=-1\n"
		"0040:  LOAD           1         calls=[ main=[ \"hello\" \"o\" ] ]  stack=[ \"o\" ] sp=0\n"
		"0043:  PRINT                    calls=[ main=[ \"hello\" \"o\" ] ]  stack=[ ] sp=-1\n"
		"0044:  SFREE          1         calls=[ main=[ \"hello\" ? ] ]  stack=[ ] sp=-1\n"
		"0047:  SFREE          0         calls=[ main=[ ? ? ] ]  stack=[ ] sp=-1\n"
		"0050:  HALT                     calls=[ main=[ ? ? ] ]  stack=[ ] sp=-1\n";
	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void br() {
	char *code =
	"0 strings\n"
	"1 functions maxaddr=7\n"
	"	0: 4/main\n"
	"5 instr, 13 bytes\n"
	"	BR 6\n"		// skip over halt at address 5
	"	HALT\n"
	"	ICONST 99\n"
	"	PRINT\n"
	"	HALT\n";

	char *expected_output = "99\n";
	char *expected_trace =
	"0000:  BR             6         calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
	"0006:  ICONST         99        calls=[ main=[ ] ]  stack=[ 99 ] sp=0\n"
	"0011:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
	"0012:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";
	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

void brf() {
	char *code =
	"0 strings\n"
	"1 functions maxaddr=7\n"
	"	0: 4/main\n"
	"8 instr, 24 bytes\n"
	"	ICONST 99\n"
	"	ICONST 99\n"
	"	INEQ\n"
	"	BRF 17\n"		// skip over halt at address 16
	"	HALT\n"
	"	ICONST 99\n"
	"	PRINT\n"
	"	HALT\n";

	char *expected_output = "99\n";
	char *expected_trace =
	"0000:  ICONST         99        calls=[ main=[ ] ]  stack=[ 99 ] sp=0\n"
	"0005:  ICONST         99        calls=[ main=[ ] ]  stack=[ 99 99 ] sp=1\n"
	"0010:  INEQ                     calls=[ main=[ ] ]  stack=[ false ] sp=0\n"
	"0011:  BRF            17        calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
	"0017:  ICONST         99        calls=[ main=[ ] ]  stack=[ 99 ] sp=0\n"
	"0022:  PRINT                    calls=[ main=[ ] ]  stack=[ ] sp=-1\n"
	"0023:  HALT                     calls=[ main=[ ] ]  stack=[ ] sp=-1\n";
	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

/*
 * var i = 0
 * while ( i<5 ) {i = i + 1 }
 * print(i)
 */
void while_stat() {
	char *code =
		"0 strings\n"
		"1 functions maxaddr=0\n"
		"	0: 4/main\n"
		"15 instr, 47 bytes\n"
		// main @ 0
		"	LOCALS 1\n"			// 0
		"	ICONST 0\n"			// 3
		"	STORE 0\n"			// 8
		// while
		"	LOAD 0\n"			// 11
		"	ICONST 5\n"			// 14
		"	ILT\n"				// 19
		"	BRF 42\n"			// 20 jump out if i>=10
		"	LOAD 0\n"			// 25
		"	ICONST 1\n"			// 28
		"	IADD\n"				// 31
		"	STORE 0\n"			// 34
		"	BR 11\n"			// 37
		// after loop:
		"	LOAD 0\n"			// 42
		"	PRINT\n"			// 45
		"	HALT\n";			// 48
	char *expected_output = "5\n";
	char *expected_trace =
		"0000:  LOCALS         1         calls=[ main=[ ? ] ]  stack=[ ] sp=-1\n"
		"0003:  ICONST         0         calls=[ main=[ ? ] ]  stack=[ 0 ] sp=0\n"
		"0008:  STORE          0         calls=[ main=[ 0 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 0 ] ]  stack=[ 0 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 0 ] ]  stack=[ 0 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 0 ] ]  stack=[ true ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 0 ] ]  stack=[ ] sp=-1\n"
		"0025:  LOAD           0         calls=[ main=[ 0 ] ]  stack=[ 0 ] sp=0\n"
		"0028:  ICONST         1         calls=[ main=[ 0 ] ]  stack=[ 0 1 ] sp=1\n"
		"0033:  IADD                     calls=[ main=[ 0 ] ]  stack=[ 1 ] sp=0\n"
		"0034:  STORE          0         calls=[ main=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0037:  BR             11        calls=[ main=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 1 ] ]  stack=[ 1 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 1 ] ]  stack=[ true ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 1 ] ]  stack=[ ] sp=-1\n"
		"0025:  LOAD           0         calls=[ main=[ 1 ] ]  stack=[ 1 ] sp=0\n"
		"0028:  ICONST         1         calls=[ main=[ 1 ] ]  stack=[ 1 1 ] sp=1\n"
		"0033:  IADD                     calls=[ main=[ 1 ] ]  stack=[ 2 ] sp=0\n"
		"0034:  STORE          0         calls=[ main=[ 2 ] ]  stack=[ ] sp=-1\n"
		"0037:  BR             11        calls=[ main=[ 2 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 2 ] ]  stack=[ 2 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 2 ] ]  stack=[ 2 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 2 ] ]  stack=[ true ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 2 ] ]  stack=[ ] sp=-1\n"
		"0025:  LOAD           0         calls=[ main=[ 2 ] ]  stack=[ 2 ] sp=0\n"
		"0028:  ICONST         1         calls=[ main=[ 2 ] ]  stack=[ 2 1 ] sp=1\n"
		"0033:  IADD                     calls=[ main=[ 2 ] ]  stack=[ 3 ] sp=0\n"
		"0034:  STORE          0         calls=[ main=[ 3 ] ]  stack=[ ] sp=-1\n"
		"0037:  BR             11        calls=[ main=[ 3 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 3 ] ]  stack=[ 3 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 3 ] ]  stack=[ 3 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 3 ] ]  stack=[ true ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 3 ] ]  stack=[ ] sp=-1\n"
		"0025:  LOAD           0         calls=[ main=[ 3 ] ]  stack=[ 3 ] sp=0\n"
		"0028:  ICONST         1         calls=[ main=[ 3 ] ]  stack=[ 3 1 ] sp=1\n"
		"0033:  IADD                     calls=[ main=[ 3 ] ]  stack=[ 4 ] sp=0\n"
		"0034:  STORE          0         calls=[ main=[ 4 ] ]  stack=[ ] sp=-1\n"
		"0037:  BR             11        calls=[ main=[ 4 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 4 ] ]  stack=[ 4 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 4 ] ]  stack=[ 4 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 4 ] ]  stack=[ true ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 4 ] ]  stack=[ ] sp=-1\n"
		"0025:  LOAD           0         calls=[ main=[ 4 ] ]  stack=[ 4 ] sp=0\n"
		"0028:  ICONST         1         calls=[ main=[ 4 ] ]  stack=[ 4 1 ] sp=1\n"
		"0033:  IADD                     calls=[ main=[ 4 ] ]  stack=[ 5 ] sp=0\n"
		"0034:  STORE          0         calls=[ main=[ 5 ] ]  stack=[ ] sp=-1\n"
		"0037:  BR             11        calls=[ main=[ 5 ] ]  stack=[ ] sp=-1\n"
		"0011:  LOAD           0         calls=[ main=[ 5 ] ]  stack=[ 5 ] sp=0\n"
		"0014:  ICONST         5         calls=[ main=[ 5 ] ]  stack=[ 5 5 ] sp=1\n"
		"0019:  ILT                      calls=[ main=[ 5 ] ]  stack=[ false ] sp=0\n"
		"0020:  BRF            42        calls=[ main=[ 5 ] ]  stack=[ ] sp=-1\n"
		"0042:  LOAD           0         calls=[ main=[ 5 ] ]  stack=[ 5 ] sp=0\n"
		"0045:  PRINT                    calls=[ main=[ 5 ] ]  stack=[ ] sp=-1\n"
		"0046:  HALT                     calls=[ main=[ 5 ] ]  stack=[ ] sp=-1\n";

	vm = run(code, true);

	assert_str_equal(expected_output, vm->output);

	diff = strdiff(expected_trace, vm->trace, 10000);
	assert_str_equal("", diff);
}

int main(int argc, char *argv[]) {
	c_unit_setup = setup;
	c_unit_teardown = teardown;

//	test(hello);
//	test(locals);
//	test(sfree);
//	test(slen);
//	test(iadd);
//	test(isub_pos);
//	test(isub_neg);
//	test(imul);
//	test(idiv);
//	test(ieq);
//	test(icmp);
//	test(sadd);
//	test(i2s);
//	test(seq);
//	test(scmp);
//	test(sindex);
//	test(br);
//	test(brf);
	test(while_stat);

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
	printf("exec\n");
	vm_exec(vm,trace);
//	printf("my output: %s\n", vm->output);
	return vm;
}
