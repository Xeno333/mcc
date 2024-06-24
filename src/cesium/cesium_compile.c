//cesium_parse.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"





static char sep_chars[] = ":,. |\\~`!@#$%%^&*()-_=+[{}];\"'<>?/";


static bool contains(char* str, const char c) {
	for (size_t p = 0; str[p] != 0; p++) {
		if (str[p] == c)
			return true;
	}
	return false;
}


static char* get_part(char* in, size_t p) {
	size_t c = 0;
	do {
		c++;
	} while ((in[p+c] != 0) && (!contains(sep_chars, in[p+c])));

	char* s = malloc(c + 1);
	
	if (c > 0)
		memcpy(s, in+p, c);

	s[c] = 0;

	return s;
}







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


	size_t p = 0;
	char* part = NULL;
	while(true) {
		if (part != NULL) 
			free(part);
			
		part = get_part(fin, p);
		if (part == NULL){
			rc = 1;
			goto done;
		}

		if (part[0] == 0) 
			break;

		p += strlen(part);

		printf("%s ", part);
	}

    done:
		if (parse_fn != NULL) {
			free(parse_fn);
			parse_fn = NULL;
		}
		return rc;
}
