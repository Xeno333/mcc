//cesium_parse.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"




static char* fin = NULL;
static size_t cp = 0L;

static bool eof = false;

static char* get_fin_line() {
    if (cp >= strlen(fin)) {
	eof = true;
	return NULL;
    }
    size_t p = cp;
    for (; ((fin[p] != '\n') && (fin[p] != '\0') && (fin[p] != ';')); p++) {
    }

    char* line = malloc(p - cp + 1);
    if (line == NULL)
        return NULL;

    p = 0;
    for (; ((fin[cp] != '\n') && (fin[cp] != '\0') && (fin[cp] != ';')); cp++, p++) line[p] = fin[cp];
    line[p] = '\0';

    //pass term char
    cp++;

    return line;
}


int parse_file(const compiler_args_t args) {
    //return code
    int rc = 0;
    char* line = NULL;

    //open the parse file
    char* parse_fn = malloc(strlen(args.in_file) + strlen(MMC_parsing_file_extension) + sizeof(char));
    strcpy(parse_fn, args.in_file);
    strcat(parse_fn, MMC_parsing_file_extension);
    FILE* parse = fopen(parse_fn, "w");
    if (parse == NULL) {
	printself();
	printf("Could not open '%s' in write mode!\n", parse_fn);
        rc = -1;
        goto done;
    }

    //close
    fclose(parse);
    parse = NULL;

    //reopen in append mode
    parse = fopen(parse_fn, "a");
    if (parse == NULL) {
	printself();
	printf("Could not open '%s' in append mode!\n", parse_fn);
        rc = -1;
        goto done;
    }

    //fget
    fin = fget(args.in_file);
    if (fin == NULL) {
	    error("Could read '", true);
        error(args.in_file, false);
        error("'!\n", false);
        rc = -1;
        goto done;
    }


    //parse
    while (true) {
        line = get_fin_line();
        if (eof == true)
            break;
            
        if (line == NULL) {
            rc = -1;
            goto done;
        }

        //skip whitespaces
        char* uline = NULL;
        size_t p = 0;
        for (; ((line[p] != '\0') && ((line[p] == '\t') || (line[p] == ' '))); p++);
        uline = line + p;

        if (uline[0] == '\0')
            continue;


        if (strncmp(uline, "#", strlen("#")) == 0) {
            if (strncmp((uline + sizeof(char)), "import ", strlen("import ")) == 0) {
                //do import
            }
            //Preprocesser
        }
        else if (strncmp(uline, "break", strlen("break")) == 0) {
        }
        else if (strncmp(uline, "continue", strlen("continue")) == 0) {
        }


        //printf("%s\n", line);
        free(line);
        line = NULL;
    }


    //end
    rc = 0;
    done:
        if (line != NULL) {
            free(line);
        }

        if (fin != NULL) {
            free(fin);
        }

        if (parse_fn != NULL) {
            free(parse_fn);
            parse_fn = NULL;
        }
        if (parse != NULL) {
            fclose(parse);
            parse = NULL;
        }

        return rc;
}
