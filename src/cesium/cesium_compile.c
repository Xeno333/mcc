//cesium_parse.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"

/*


static char* fin = NULL;
static size_t cp = 0L;

static bool eof = false;

static char* get_line(char* f) {
	if (cp >= strlen(f)) {
		eof = true;
		return NULL;
	}
	size_t p = cp;
	for (; ((f[p] != '\n') && (f[p] != '\0') && (f[p] != ';')); p++) {
	}

	char* line = malloc(p - cp + 1);
	if (line == NULL)
		return NULL;

	p = 0;
	for (; ((f[cp] != '\n') && (f[cp] != '\0') && (f[cp] != ';')); cp++, p++) line[p] = f[cp];
	line[p] = '\0';

	//pass term char
	cp++;

	return line;
}*/


int compile_file(const compiler_args_t args) {
	//return code
	int rc = 0;

	/*//open the parse file
	char* parse_fn = malloc(strlen(args.in_file) + strlen(MMC_parsing_file_extension) + 1);
	strcpy(parse_fn, args.in_file);
	strcat(parse_fn, MMC_parsing_file_extension);
	FILE* file = fopen(parse_fn, "r");
	if (parse == NULL) {
	printself();
	printf("Could not open '%s' in write mode!\n", parse_fn);
		rc = -1;
		goto done;
	}*/

    done:
		return rc;
}
