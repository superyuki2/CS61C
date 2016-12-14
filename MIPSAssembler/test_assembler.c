#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "src/utils.h"
#include "src/tables.h"
#include "src/translate_utils.h"
#include "src/translate.h"

const char* TMP_FILE = "test_output.txt";
const int BUF_SIZE = 1024;

/****************************************
 *  Helper functions 
 ****************************************/

int do_nothing() {
    return 0;
}

int init_log_file() {
    set_log_file(TMP_FILE);
    return 0;
}

int check_lines_equal(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TMP_FILE, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

/****************************************
 *  Test cases for translate_utils.c 
 ****************************************/

void test_translate_reg() {
    CU_ASSERT_EQUAL(translate_reg("$0"), 0);
    CU_ASSERT_EQUAL(translate_reg("$at"), 1);
    CU_ASSERT_EQUAL(translate_reg("$v0"), 2);
    CU_ASSERT_EQUAL(translate_reg("$a0"), 4);
    CU_ASSERT_EQUAL(translate_reg("$a1"), 5);
    CU_ASSERT_EQUAL(translate_reg("$a2"), 6);
    CU_ASSERT_EQUAL(translate_reg("$a3"), 7);
    CU_ASSERT_EQUAL(translate_reg("$t0"), 8);
    CU_ASSERT_EQUAL(translate_reg("$t1"), 9);
    CU_ASSERT_EQUAL(translate_reg("$t2"), 10);
    CU_ASSERT_EQUAL(translate_reg("$t3"), 11);
    CU_ASSERT_EQUAL(translate_reg("$s0"), 16);
    CU_ASSERT_EQUAL(translate_reg("$s1"), 17);
    CU_ASSERT_EQUAL(translate_reg("$3"), -1);
    CU_ASSERT_EQUAL(translate_reg("asdf"), -1);
    CU_ASSERT_EQUAL(translate_reg("hey there"), -1);
}

void test_translate_num() {
    long int output;

    CU_ASSERT_EQUAL(translate_num(&output, "35", -1000, 1000), 0);
    CU_ASSERT_EQUAL(output, 35);
    CU_ASSERT_EQUAL(translate_num(&output, "145634236", 0, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 145634236);
    CU_ASSERT_EQUAL(translate_num(&output, "0xC0FFEE", -9000000000, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 12648430);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 72), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 71), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 72, 150), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 73, 150), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "35x", -100, 100), -1);
}

/****************************************
 *  Test cases for tables.c 
 ****************************************/

void test_table_1() {
    int retval;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    retval = add_to_table(tbl, "abc", 8);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "efg", 12);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 24); 
    CU_ASSERT_EQUAL(retval, -1); 
    retval = add_to_table(tbl, "bob", 14); 
    CU_ASSERT_EQUAL(retval, -1); 

    retval = get_addr_for_symbol(tbl, "abc");
    CU_ASSERT_EQUAL(retval, 8); 
    retval = get_addr_for_symbol(tbl, "q45");
    CU_ASSERT_EQUAL(retval, 16); 
    retval = get_addr_for_symbol(tbl, "ef");
    CU_ASSERT_EQUAL(retval, -1);
    
    free_table(tbl);

    char* arr[] = { "Error: name 'q45' already exists in table.",
                    "Error: address is not a multiple of 4." };
    check_lines_equal(arr, 2);

    SymbolTable* tbl2 = create_table(SYMTBL_NON_UNIQUE);
    CU_ASSERT_PTR_NOT_NULL(tbl2);

    retval = add_to_table(tbl2, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl2, "q45", 24); 
    CU_ASSERT_EQUAL(retval, 0);

    free_table(tbl2);

}

void test_table_2() {
    int retval, max = 100;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    char buf[10];
    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = add_to_table(tbl, buf, 4 * i);
        CU_ASSERT_EQUAL(retval, 0);
    }

    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = get_addr_for_symbol(tbl, buf);
        CU_ASSERT_EQUAL(retval, 4 * i);
    }

    free_table(tbl);
}

/****************************************
 *  Add your test cases here
 ****************************************/
/****************************************
 *  Test cases for translate.c
 ****************************************/
void test_rtype() {
	FILE* file = fopen(TMP_FILE, "w");
	char *args1[3] = {"$s0", "$s1", "$0"};
	char *args2[3] = {"$t1", "$s1", "$s0"};
	char *args3[3] = {"$s0", "$s1", "$s2"};
	char *args4[3] = {"$s1", "$s2", "$s3"};
	char *args5[3] = {"$s5", "$s1", "$s2"};
	int addu = translate_inst(fstout, "addu", args1, 3, 0, NULL, NULL);
	CU_ASSERT_EQUAL(addu, 0);
	int or = translate_inst(fstout, "or", args2, 3, 0, NULL, NULL);
	CU_ASSERT_EQUAL(or, 0);
	int slt = translate_inst(fstout, "slt", args3, 3, 0, NULL, NULL);
	CU_ASSERT_EQUAL(slt, 0);
	int sltu = translate_inst(fstout, "sltu", args4, 3, 0, NULL, NULL);
	CU_ASSERT_EQUAL(sltu, 0);
	int error = translate_inst(fstout, "addu", args5, 3, 0, NULL, NULL);
	CU_ASSERT_EQUAL(error, -1);
	
	fclose(file);
	char* ans[] = {"02208021"
			"02304825"
			"0232802a"
			"0253882b"};
	check_lines_equal(ans, 4);
}
void test_shift() {
	FILE* file = fopen(TMP_FILE, "w");
		char *args1[3] = {"$s1", "$s2", "5"};
		char *args2[3] = {"$t1", "$s1", "100"};
		char *args2[3] = {"$s1", "$t1", "$0"};
	    int sll1 = translate_inst(file, "sll", args1, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(sll1, 0);
	    int sll2 = translate_inst(file, "sll", args2, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(sll2, -1);
	    int sll3 = translate_inst(file, "sll", args3, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(sll3, -1);

	    fclose(file);
	    char* ans[] = {"00128940"};
	    check_lines_equal(ans, 1);
}
void test_jr() {
	FILE* file = fopen(TMP_FILE, "w");
		char *args1[3] = {"$s0"};
		char *args2[3] = {"$0"};
		char *args3[3] = {"10"};
		int jr1 = translate_inst(file, "jr", args1, 1, 0, NULL, NULL);
		CU_ASSERT_EQUAL(jr1, 0);
		int jr2 = translate_inst(file, "jr", args2, 1, 0, NULL, NULL);
		CU_ASSERT_EQUAL(jr2, 0);
		int jr3 = translate_inst(file, "jr", args3, 1, 0, NULL, NULL);
		CU_ASSERT_EQUAL(jr3, -1);
		
		fclose(file);
		char* hexcode[] = {"02000008", "00000008"};
		check_lines_equal(hexcode, 2);
	
}
void test_addiu() {
	FILE* file = fopen(TMP_FILE, "w");
		char *args1[3] = {"$s1", "$s2", "5"};
		char *args2[3] = {"$t1", "$s1", "100"};
		char *args2[3] = {"$s1", "$t1", "-1"};
	    int addiu1 = translate_inst(file, "sll", args1, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(addiu1, 0);
	    int addiu2 = translate_inst(file, "sll", args2, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(addiu2, 0);
	    int addiu3 = translate_inst(file, "sll", args3, 3, 0, NULL, NULL);
	    CU_ASSERT_EQUAL(addiu3, 0);

	    fclose(file);
	    char* hexcode[] = {"26510005",
	    		"26290064",
	    		"2531ffff"};
	    check_lines_equal(hexcode, 3);
	
}
void test_lui() {
    FILE* file = fopen(TMP_FILE, "w");
    char *args1[2] = {"$a0", "$0"};
    char *args2[2] = {"$s0", "65535"};
    char *args3[2] = {"$a0", "65536"};
    char *args4[2] = {"$v0", "-1"};

    int lui1 = translate_inst(file, "lui", args1, 2, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lui1, 0);
    int lui2 = translate_inst(file, "lui", args2, 2, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lui2, 0);
    int lui3 = translate_inst(file, "lui", args3, 2, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lui3, -1);
    int lui4 = translate_inst(file, "lui", args4, 2, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lui4, -1);

    fclose(file);
    char* hexcode[] = {"3c040000",
    		"3c10ffff"};
    check_lines_equal(hexcode, 2);
	
}
void test_ori() {
    FILE* file = fopen(TMP_FILE, "w");
    char *args1[3] = {"$s0", "$t2", "1"};
    char *args2[3] = {"$t2", "$t1", "65535"};
    char *args3[3] = {"$a0", "$zero", "-1"};

    int ori1 = translate_inst(file, "ori", args1, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(ori1, 0);
    int ori2 = translate_inst(file, "ori", args2, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(ori2, 0);
    int ori3 = translate_inst(file, "ori", args3, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(ori3, -1);
    fclose(file);
    char* hexcode[] = {"35500001",
    		"352affff"};
    check_lines_equal(hexcode, 2);
}
void test_mem() {
    FILE* file = fopen(TMP_FILE, "w");
    
    char *args1[3] = {"$a0", "3", "$s1"};
    char *args2[3] = {"$t0", "2", "$ra"};
    char *args3[3] = {"$a0", "3", "$a1"};
    char *args4[3] = {"$v0", "$s1", "$s2"};
    char *args5[3] = {"$t2", "1000", "$a1"};
    char *args6[3] = {"$s2", "5", "$a1"};

    int lb = translate_inst(file, "lb", args1, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lb, 0);
    int lw = translate_inst(file, "lw", args2, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(lw, 0);
    int sw = translate_inst(file, "sw", args3, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(sw, 0);
    int mult = translate_inst(file, "mult", args4, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(mult, 0);
    int mflo = translate_inst(file, "mflo", args5, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(mflo, 0);
    int mfhi = translate_inst(file, "mfhi", args6, 3, 0, NULL, NULL);
    CU_ASSERT_EQUAL(mfhi, 0);
    fclose(file);

//    fclose(file);
//    char* hexcode[] = {};
//    check_lines_equal(hexcode, 6);
}
void test_branch() {
    FILE* file = fopen("branchtest.txt", "w");

SymbolTable* table = create_table(1);
add_to_table(table, "Label", 1000000);
char *args1[3] = {"$a0", "$t0", "Label"};
char *args2[3] = {"$a0", "$0", "Label"};
char *args3[3] = {"$a0", "$t0", "hello"};

int beq = translate_inst(file, "beq", args1, 3, 1000004, table, NULL);
CU_ASSERT_EQUAL(beq, 0);
int bne = translate_inst(file, "bne", args2, 3, 1000008, table, NULL);
CU_ASSERT_EQUAL(bne, 0);
int error = translate_inst(file, "bne", args3, 3, 1000000, table, NULL);
CU_ASSERT_EQUAL(error, -1);

fclose(file);
char * hexcode[] = {"1088fffe",
            "1400fff"};
check_lines_equal(hexcode, 2);
}


void test_jump() {
	FILE* file = fopen(TMP_FILE, "w");

    SymbolTable* table = create_table(1);
    add_to_table(table, "Label", 1000000);
    char *args1[3] = {"$a0", "$t0", "Label"};
    char *args2[3] = {"$a0", "$0", "Label"};
    char *args3[3] = {"$a0", "$t0", "hello"};
    
    int beq = translate_inst(file, "beq", args1, 1, 1000000, NULL, table);
    CU_ASSERT_EQUAL(beq, 0);
    int bne = translate_inst(file, "bne", args2, 1, 1000000, NULL, table);
    CU_ASSERT_EQUAL(bne, 0);
    int error = translate_inst(file, "bne", args3, 1, 1000000, NULL, table);
    CU_ASSERT_EQUAL(error, -1);

    fclose(file);
    char * hexcode[] = {"1088ffff",
    		"1400fffe"}
    check_lines_equal(hexcode, 2);
}
void test_write_pass_one() {
    FILE* file = fopen(TMP_FILE, "w");

    SymbolTable* table = create_table(1);
    add_to_table(t1, "Label", 1000000);
    char *args1[2] = {"$s1", "5"};
    char *args2[2] = {"$s2", "100000"};
    char *args3[2] = {"$s1", "$s2"};
    char *args4[3] = {"$v0", "s1", "s2"};
    char *args5[3] = {"$s1", "$t2", "Label"};
    char *args6[3] = {"$s2", "Label"};

    int li1 = write_pass_one(file, "li", args1, 2);
    CU_ASSERT_EQUAL(li1, 1);
    int li2 = write_pass_one(file, "li", args2, 2);
    CU_ASSERT_EQUAL(li2, 2);
    int move = write_pass_one(file, "move", args3, 2);
    CU_ASSERT_EQUAL(move, 1);
    int rem = write_pass_one(file, "rem", args4, 3);
    CU_ASSERT_EQUAL(rem, 2);
    int bge = write_pass_one(file, "bge", args5, 3);
    CU_ASSERT_EQUAL(bge, 2);
    int bnez = write_pass_one(file, "bnez", args6, 2);
    CU_ASSERT_EQUAL(bnez, 1);

    fclose(file);
//    char* hexcode[] = {"addiu $s1, $0, 5", "lui $at, 1", "ori $s2, $at, 34464", "addu $s1, $s2, $0",
//    	    "div $s1, $s2", "mfhi $v0", "slt $at, $s1, $t2", "beq $at, $0, Label", "bne $s2, $0, Label"};
    char * hexcode[] = {"addiu $s1, $0, 5",
    		"lui $at, 1",
    		"ori $s2, $at, 34464",
    		"addu $s1, $s2, $0",
    		"div $s1, $s2",
    		"mfhi $v0",
    		"slt $at, $s1, $t2",
    		"beq $at, $0, Label",
    		"bne $s2, $0, Label"}
    check_lines_equal(hexcode, 9);
	
}



int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL, pSuite3 = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* Suite 1 */
    pSuite1 = CU_add_suite("Testing translate_utils.c", NULL, NULL);
    if (!pSuite1) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_reg", test_translate_reg)) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_num", test_translate_num)) {
        goto exit;
    }

    /* Suite 2 */
    pSuite2 = CU_add_suite("Testing tables.c", init_log_file, NULL);
    if (!pSuite2) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_1", test_table_1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_2", test_table_2)) {
        goto exit;
    }
    
    pSuite3 = CU_add_suite("Testing translate.c", init_log_file, NULL);
    if (!pSuite3) {
    	goto exit;
    }
    if (!CU_add_test(pSuite3, "test_rtype", test_rtype)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_shift", test_shift)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_jr", test_jr)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_addiu", test_addiu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_ori", test_ori)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_lui", test_lui)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_mem", test_mem)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_branch", test_branch)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_jump", test_jump)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_write_pass_one", test_write_pass_one)) {
        goto exit;
    }
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}
