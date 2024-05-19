//hrriscasm.c

//system
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"


FILE* fin = NULL;
FILE* fout = NULL;
FILE* foutobj = NULL;

typedef struct {
    char* label;
    char* nonasm;
    char* operator;
    char* opsize;
    char* operand1;
    char* operand2;
    char* operand3;
    char* operand4;
} instruction;



typedef struct {
    char lable_name[256];
    unsigned long long lable_offset;
} lable_map_entry;


enum {
    set_up_labels = 0,
    get_labels = 1,
    compile_code = 2,
} flag = set_up_labels;


struct {
    unsigned long long main_offset;
    unsigned long long data_offset;
    unsigned long long rodata_offset;
}__attribute__((packed)) Output_header = {0, 0, 0};


instruction token = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

lable_map_entry* lable_map;
int lable_map_pointer = 0;
int lable_map_size = 0;

unsigned char bytecode_instruction[64];
unsigned long long bytecode_instruction_length = 0;

unsigned long long offset = 0;


int printf_instruction(instruction* instruction) {
    printself();
    printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n", instruction->label, instruction->operator, instruction->opsize, instruction->operand1, instruction->operand2, instruction->operand3, instruction->operand4);
    return 0;
}

void close(int rc) {
    if (lable_map != NULL) free(lable_map);

    // Free memory allocated for strings inside the token struct
    if (token.label != NULL) free(token.label);
    if (token.nonasm != NULL) free(token.nonasm);
    if (token.operator != NULL) free(token.operator);
    if (token.opsize != NULL) free(token.opsize);
    if (token.operand1 != NULL) free(token.operand1);
    if (token.operand2 != NULL) free(token.operand2);
    if (token.operand3 != NULL) free(token.operand3);
    if (token.operand4 != NULL) free(token.operand4);

    // Close file pointers and exit
    if (fin != NULL)
        fclose(fin);
    if (fout != NULL)
        fclose(fout);
    if (foutobj != NULL)
        fclose(foutobj);
    exit(rc);
}


typedef struct {
    char reg[6];
    unsigned char regid;
} register_layout;

register_layout registers[32] = {
    {"r0", 0},
    {"r1", 1},
    {"r2", 2},
    {"r3", 3},
    {"r4", 4},
    {"r5", 5},
    {"r6", 6},
    {"r7", 7},
    {"r8", 8},
    {"r9", 9},
    {"r10", 10},
    {"r11", 11},
    {"r12", 12},
    {"r13", 13},
    {"r14", 14},
    {"r15", 15},
    {"[r0]", 0},
    {"[r1]", 1},
    {"[r2]", 2},
    {"[r3]", 3},
    {"[r4]", 4},
    {"[r5]", 5},
    {"[r6]", 6},
    {"[r7]", 7},
    {"[r8]", 8},
    {"[r9]", 9},
    {"[r10]", 10},
    {"[r11]", 11},
    {"[r12]", 12},
    {"[r13]", 13},
    {"[r14]", 14},
    {"[r15]", 15}
};

unsigned long long rais(unsigned long long base,int p) {
    unsigned long long out = 1;

    for (int i = 0; i < p; ++i) out = out * base;

    return(out);
}

unsigned char get_regid(char* reg) {
    int i = -1;
    while (strcmp(registers[++i].reg, reg) != 0);
    return registers[i].regid;
}

unsigned long long* get_const(char* consti) {
    static unsigned long long constv = 0;

    if ((consti[0] == '0') && (consti[1] == 'x')) {
        constv = strtoull(consti + 2, NULL, 16);
    }
    else if ((consti[0] - '0' >= 0) && (consti[0] - '0' < 10) ) {
        constv = strtoull(consti, NULL, 10);
        //Buggy
    }
    else {
        for (int i = 0; i < lable_map_pointer; i++) {
            if (strcmp(lable_map[i].lable_name, consti) == 0) {
                constv = lable_map[i].lable_offset;
                break;
            }
        }
    }

    return &constv;
}




int tokenize(const char* line, int line_number) {
    token = (instruction){0, 0, 0, 0, 0, 0, 0};
    char str[256];

    int i = 0;
    for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    if (line[i] == ';') return 0;
    if (line[i] == '$') {
        int i = 0;
        while (line[++i] != 0);
        token.nonasm = malloc(sizeof(char) * (i + 1));
        strcpy(token.nonasm, line);
        token.nonasm[i-1] = 0;
        return 0;
    }

    //label
    for (int i2 = 0, i3 = i; (line[i3] != '\n') && (line[i3] != ' ') && (line[i3] != '\t'); i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, lable (bad instruction) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if (line[i3] == ':') {
            i = i3;
            str[i2] = '\0';
            token.label = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.label, str);
            i++;
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //operator
    for (int i2 = 0, i3 = i; (line[i3] != '\n'); i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, operator (bad instruction) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if ((line[i3] == ' ') || (line[i3] == '\t')) {
            i = i3;
            str[i2] = 0;
            token.operator = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.operator, str);
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //opsize
    if ((line[i] == 'q') || (line[i] == 'd') || (line[i] == 'w') || (line[i] == 'b')) {
        token.opsize = malloc(sizeof(char) + 1);
        token.opsize[0] = line[i];
        token.opsize[1] = '\0';
        i++;
        if ((line[i] != ' ') && (line[i] != '\t')) {
            printself();
            printf("Bad instruction opsize on line: %d\n%s\n", line_number, line);
            close(1);
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //opr1
    for (int i2 = 0, i3 = i; 0 == 0; i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, operand1 (bad operand) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if ((line[i3] == ' ') || (line[i3] == '\t')) {
            printself();
            printf("Error on line: %d, operand1 (bad operand) maybe missing ','\n%s\n", line_number, line);
            close(1);
        }
        if ((line[i3] == ',') || (line[i3] == '\n')) {
            i = i3;
            str[i2] = '\0';
            token.operand1 = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.operand1, str);
            if (line[i] != '\n') {
                i++;
            }
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //opr2
    for (int i2 = 0, i3 = i; 0 == 0; i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, operand2 (bad operand) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if ((line[i3] == ' ') || (line[i3] == '\t')) {
            printself();
            printf("Error on line: %d, operand2 (bad operand) maybe missing ','\n%s\n", line_number, line);
            close(1);
        }
        if ((line[i3] == ',') || (line[i3] == '\n')) {
            i = i3;
            str[i2] = '\0';
            token.operand2 = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.operand2, str);
            if (line[i] != '\n') {
                i++;
            }
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //opr3
    for (int i2 = 0, i3 = i; 0 == 0; i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, operand3 (bad operand) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if ((line[i3] == ' ') || (line[i3] == '\t')) {
            printself();
            printf("Error on line: %d, operand3 (bad operand) maybe missing ','\n%s\n", line_number, line);
            close(1);
        }
        if ((line[i3] == ',') || (line[i3] == '\n')) {
            i = i3;
            str[i2] = '\0';
            token.operand3 = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.operand3, str);
            if (line[i] != '\n') {
                i++;
            }
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    //opr4
    for (int i2 = 0, i3 = i; 0 == 0; i2++, i3++) {
        if (i2 == sizeof(str)) {
            printself();
            printf("Error on line: %d, operand4 (bad operand) is too long!\n%s\n", line_number, line);
            close(1);
        }
        str[i2] = line[i3];
        if ((line[i3] == ' ') || (line[i3] == '\t')) {
            printself();
            printf("Error on line: %d, operand4 (bad operand) maybe missing ','\n%s\n", line_number, line);
            close(1);
        }
        if ((line[i3] == ',') || (line[i3] == '\n')) {
            i = i3;
            str[i2] = '\0';
            token.operand4 = malloc(sizeof(char) * (i2 + 1));
            strcpy(token.operand4, str);
            if (line[i] != '\n') {
                i++;
            }
            break;
        }
    } for (; (line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n'); i++) {
        if (line[i] == '\n') {
            return 0;
        }
    }

    return 0;
}




void compile_xtn(int line_number) {
    if (strcmp(token.operand1, "vmfc") == 0) {
        if (token.operand2 == NULL) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        unsigned long long num = *get_const(token.operand2);
        unsigned char* num_array = (unsigned char*)&num;

        bytecode_instruction[bytecode_instruction_length++] = 0xb;//size

        bytecode_instruction[bytecode_instruction_length++] = 0x00; //operator

        bytecode_instruction[bytecode_instruction_length++] = num_array[0];
        bytecode_instruction[bytecode_instruction_length++] = num_array[1];
        bytecode_instruction[bytecode_instruction_length++] = num_array[2];
        bytecode_instruction[bytecode_instruction_length++] = num_array[3];
        bytecode_instruction[bytecode_instruction_length++] = num_array[4];
        bytecode_instruction[bytecode_instruction_length++] = num_array[5];
        bytecode_instruction[bytecode_instruction_length++] = num_array[6];
        bytecode_instruction[bytecode_instruction_length++] = num_array[7];
    }
    
    else {
        printself();
        printf("Error on line: %d, bad instruction: %s %s!\n", line_number, token.operator, token.operand1);
        close(1);
    }
}



int compile_istruction(int line_number) {
    bytecode_instruction_length = 0;
    unsigned char opcode = 0;

    if (token.opsize != NULL) {
        switch (token.opsize[0]) {
            case 'q':
                opcode = opcode | 0b11000000;
                break;
            case 'd':
                opcode = opcode | 0b10000000;
                break;
            case 'w':
                opcode = opcode | 0b01000000;
                break;
            case 'b':
                opcode = opcode | 0b00000000;
                break;
            default:
                printself();
                printf("Error on line: %d, bad operand size!\n", line_number);
                close(1);
        }
    }
    if ((token.operand1 != NULL) && (token.operand1[0] == '[')) opcode = opcode | 0b00100000;
    if ((token.operand2 != NULL) && (token.operand2[0] == '[')) opcode = opcode | 0b00010000;


    //Compile instruct
    if (strcmp(token.operator, "mul") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = opcode & 0xf0;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "div") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 1;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "mod") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 2;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
        return 0;
    }

    else if (strcmp(token.operator, "add") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 3;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "sub") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 4;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "or") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 5;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "and") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 6;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "xor") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 7;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "not") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 != NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 8;
        bytecode_instruction[bytecode_instruction_length++] = (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "mov") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 9;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "set") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        unsigned long long operand_size = (opcode >> 6);
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xa;
        bytecode_instruction[bytecode_instruction_length++] = (get_regid(token.operand1 ) << 4);
        unsigned char* const_operand = (unsigned char*)get_const(token.operand2);
        operand_size = rais(2, operand_size);
        for (int i = 0; operand_size != i; i++) {
            bytecode_instruction[bytecode_instruction_length++] = const_operand[i];
        }
    }

    else if (strcmp(token.operator, "push") == 0) {
        if ((token.operand1 != NULL) && (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xb;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2);
    }

    else if (strcmp(token.operator, "pop") == 0) {
        if ((token.operand1 == NULL) && (token.operand2 != NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xc;
        bytecode_instruction[bytecode_instruction_length++] = (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "cmp") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xd;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand2) | (get_regid(token.operand1 ) << 4);
    }

    else if (strcmp(token.operator, "cmov") == 0) {
        if ((token.operand1 == NULL) || (token.operand2 == NULL) || (token.operand3 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xe;
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand1);
        bytecode_instruction[bytecode_instruction_length++] = get_regid(token.operand3) | (get_regid(token.operand2) << 4);
    }

    else if (strcmp(token.operator, "xtn") == 0) {
        bytecode_instruction[bytecode_instruction_length++] = (opcode & 0xf0) | 0xf;
        compile_xtn(line_number);
    }

    else if (strcmp(token.operator, "res") == 0) {
        if ((token.operand1 == NULL)) {
            printself();
            printf("Error on line: %d, bad operand!\n", line_number);
            close(1);
        }
        unsigned char* const_operand = (unsigned char*)get_const(token.operand1);
        unsigned long long operand_size = (opcode >> 6);
        operand_size = rais(2, operand_size);
        for (int i = 0; operand_size != i; i++) {
            bytecode_instruction[bytecode_instruction_length++] = const_operand[i];
        }
    }

    else {
        printself();
        printf("Error on line: %d, bad instruction: %s!\n", line_number, token.operator);
        close(1);
    }

    return 0;
}




int do_line(char* line, int line_number) {
    token = (instruction){0, 0, 0, 0, 0, 0, 0};

    //make token
    tokenize(line, line_number);

    //Size
    if (flag == set_up_labels) {
        if (token.label != NULL) {
            lable_map_size++;
            printself();
            printf("Label: %s found.\n", token.label);
        }
    }
    
    //add lable if not null
    else if (flag == get_labels) {
        if (token.label != NULL) {
            strcpy(lable_map[lable_map_pointer].lable_name, token.label);
            lable_map[lable_map_pointer].lable_offset = offset;
            lable_map_pointer++;
            printself();
            printf("Label: %s added.\n", token.label);
        }
    }

    //if not getting label table len then compile for size check
    if ((flag != set_up_labels) && (token.operator != NULL) && (token.operator[0] != '$')) {
        compile_istruction(line_number);
        if ((token.label != NULL) && (strcmp(token.label, "main") == 0)) {
            Output_header.main_offset = offset;
        }
        offset = offset + bytecode_instruction_length;
    }

    if ((flag != set_up_labels) && (token.nonasm != NULL) && (token.nonasm[0] == '$')) {
        if (strcmp(token.nonasm, "$data") == 0) {
            Output_header.data_offset = offset;
        }
        else if (strcmp(token.nonasm, "$rodata") == 0) {
            Output_header.rodata_offset = offset;
        }
    }

    return 0;
}



int hrriscasm_main(const compiler_args_t args) {
    int bin = 0;
    if (args.format == raw) 
        bin = 1;

    fin = fopen(args.in_file, "r");
    if (fin == NULL || fout == NULL) {
        printself();
        printf("Bad src file!\n");
        close(1);
    }
    fout = fopen(args.out_file, "w");
    fclose(fout);
    fout = fopen(args.out_file, "a");
    char objname[1024];
    strcpy(objname, args.out_file);
    strcat(objname, "obj");
    foutobj = fopen(objname, "w");
    fclose(foutobj);
    foutobj = fopen(objname, "a");

    if (fin == NULL || fout == NULL) {
        printself();
        printf("Real files needed! Also only 1 input and ouput for this compiler\n");
        close(1);
    }



    //Compile    
    int line = 1;
    char linein[512];

    printself();
    printf("Now parsing\n");
    do {
        if (fgets(linein, 512, fin) == NULL) {
            if (flag == compile_code) break;
            if (flag == set_up_labels) {
                line = 1;
                offset = 0;
                lable_map = malloc(lable_map_size * sizeof(lable_map_entry));
                flag = get_labels;
                printself();
                printf("\nNow entering labels\n");
            }
            else if (flag == get_labels) {
                line = 1;
                offset = 0;
                flag = compile_code;
                printself();
                printf("\nNow compiling\n");
            }
            
            rewind(fin);
            
            continue;
        }
        bytecode_instruction_length = 0;
        do_line(linein, line);

        //To file
        if (flag == compile_code) {
            fwrite(&bytecode_instruction, bytecode_instruction_length, 1, foutobj);
        }

        line++;
    } while(0 == 0);

    if (bin == 1) {
        fclose(foutobj);

        foutobj = fopen(objname, "r");
        fseek(foutobj, 0L, SEEK_END);

        unsigned long long fobjsize = ftell(foutobj);
        rewind(foutobj);

        unsigned char* obj = malloc(fobjsize);
        if (fread(obj, 1, fobjsize, foutobj) != (fobjsize * 1)) {
            printself();
            printf("An error has occored!\n");
            free(obj);
            close(-1);
        }
        fwrite(obj, fobjsize, 1, fout);
        free(obj);

    }
    else {
        fwrite(&Output_header, sizeof(Output_header), 1, fout);
        fclose(foutobj);

        foutobj = fopen(objname, "r");
        fseek(foutobj, 0L, SEEK_END);

        unsigned long long fobjsize = ftell(foutobj);
        rewind(foutobj);

        unsigned char* obj = malloc(fobjsize);
        if (fread(obj, 1, fobjsize, foutobj) != (fobjsize * 1)) {
            printself();
            printf("An error has occored!\n");
            free(obj);
            close(-1);
        }
        fwrite(obj, fobjsize, 1, fout);
        free(obj);
    }

    if (lable_map != NULL) free(lable_map);

    // Free memory allocated for strings inside the token struct
    if (token.label != NULL) free(token.label);
    if (token.nonasm != NULL) free(token.nonasm);
    if (token.operator != NULL) free(token.operator);
    if (token.opsize != NULL) free(token.opsize);
    if (token.operand1 != NULL) free(token.operand1);
    if (token.operand2 != NULL) free(token.operand2);
    if (token.operand3 != NULL) free(token.operand3);
    if (token.operand4 != NULL) free(token.operand4);

    // Close file pointers and exit
    if (fin != NULL)
        fclose(fin);
    if (fout != NULL)
        fclose(fout);
    if (foutobj != NULL)
        fclose(foutobj);
        
    return 0;
}