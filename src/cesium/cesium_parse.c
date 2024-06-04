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
}


int parse_file(const compiler_args_t args) {
	//return code
	int rc = 0;
	char* line = NULL;
	char* incfile = NULL;

	//open the parse file
	char* parse_fn = malloc(strlen(args.in_file) + strlen(MMC_parsing_file_extension) + 1);
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
	long long fin_len = strlen(fin);

	//handel includes

	//parse
	while (true) {
		line = get_line(fin);
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

		if (uline == NULL)
			continue;

		if (uline[0] == '\0')
			continue;

		//include
		if (strncmp(uline, "#import ", strlen("#import ")) == 0) {
			char* ftr = uline + strlen("#import ");
			while ((ftr[0] == ' ') || (ftr[0] == '\t'))
				ftr++;

			if (args.inc_path == NULL) {
				error("Can not include file '", true);
				error(ftr, false);
				error("'!\n", false);
				rc = -1;
				goto done;
			}

			incfile = malloc(strlen(args.inc_path) + strlen(ftr) + 1);
			size_t incfile_len = strlen(args.inc_path) + strlen(ftr) + 1;
			strncpy(incfile, args.inc_path, incfile_len);
			strncat(incfile, ftr, incfile_len - strlen(args.inc_path));
			
			//fget
			char* finc = fget(incfile);
			if (finc == NULL) {
				error("Could not read '", true);
				error(incfile, false);
				error("'!\n", false);
				rc = -1;
				goto done;
			}
			//long long finc_len = strlen(finc);

			//insert...

			char* mid = malloc(strlen(finc) + strlen(fin) + 1);

			//insert here

			memcpy(mid, fin, cp - strlen(line) - 1);
			memcpy(mid + (cp - strlen(line) - 1), finc, strlen(finc));
			memcpy(mid + (cp - strlen(line) - 1) + strlen(finc), fin + cp, strlen(fin) - cp);
			mid[(cp - strlen(line) - 1) + strlen(finc) + strlen(fin) - cp] = 0;

			cp = cp - strlen(line) - 1;

			free(fin);
			fin = mid;

			free(incfile);
			incfile = NULL;

			free(finc);
			finc = NULL;

			error("Cesium import!\n", true);
		}

		//printf("%s\n", line);
		free(line);
		line = NULL;
	}

	//add block to interate and replace ; with \n
	struct lflags_s {
		bool str : 1;
		bool comment : 1;
		bool ischar : 1;
		bool esc : 1;
	} lflags = {false, false, false, false};

	for (long long i = 0; i != fin_len; i += 1) {
		if (fin[i] == '\\')
			lflags.esc = true;

		else if ((fin[i] == '\n') && (lflags.comment == true))
			lflags.comment = false;

		else if ((fin[i] == '*') && (fin[i + 1] == '/') )
			lflags.comment = false;

		else if (fin[i] == '\'') 
			lflags.ischar = !lflags.ischar;

		else if ((fin[i] == '/') && (fin[i + 1] == '/') )
			lflags.comment = true;

		else if ((fin[i] == '/') && (fin[i + 1] == '*') )
			lflags.comment = true;

		else if (fin[i] == '"') 
			lflags.str = !lflags.str;

		else if (lflags.esc == true);
		else if (lflags.str == true);
		else if (lflags.ischar == true);
		else if (lflags.comment == true);

		else if (fin[i] == ';')
			fin[i] = '\n';
		
		if (lflags.esc == true) 
			lflags.esc = false;
	}

	//send to parse
	cp = 0L;
	eof = false;
	while (true) {
		line = get_line(fin);
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

		if (uline == NULL)
			continue;

		if (uline[0] == '\0')
			continue;

		fprintf(parse, "%s\n", uline);

		free(line);
		line = NULL;
	}

	//end
	rc = 0;
	done:
		if (line != NULL) {
			free(line);
			line = NULL;
		}
		
		if (incfile != NULL) {
			free(incfile);
			incfile = NULL;
		}

		if (fin != NULL) {
			free(fin);
			fin = NULL;
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
