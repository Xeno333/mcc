//cesium_parse.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"


int compile_file(const compiler_args_t args) {
	//return code
	int rc = 0;

	//open the parse file
	char* parse_fn = malloc(strlen(args.in_file) + strlen(MMC_parsing_file_extension) + 1);
	if (parse_fn == NULL) {
		rc = 1;
		goto done;
	}
	strcpy(parse_fn, args.in_file);
	strcat(parse_fn, MMC_parsing_file_extension);

	char* fin = fget(parse_fn);


    done:
		if (parse_fn != NULL) {
			free(parse_fn);
			parse_fn = NULL;
		}
		return rc;
}
