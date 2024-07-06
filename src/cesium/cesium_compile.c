//cesium_parse.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"





static char sep_chars[] = "\n:,. |\\~`!@#$%%^&*()-_=+[{}];\"'<>?/";


static char* get_part(char* in, size_t p) {
	size_t c = 0;
	do {
		c++;
	} while ((in[p+c] != 0) && (!contains(sep_chars, in[p+c])) && (!contains(sep_chars, in[p+c-1])));

	char* s = (char*)malloc(c + 1);
	
	if (c > 0)
		memcpy(s, in+p, c);

	s[c] = 0;

	return s;
}




int compile_file(const compiler_args_t args) {
	char err[1024];
	strcpy(err, "Mem error!");
	//return code
	int rc = 0;
	char* part = NULL;
	char* out = (char*)malloc(strlen("#if !(defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)\n\ttypedef enum {\n\t\tfalse = 0,\n\t\ttrue = 1\n\t} bool;\n#endif\n") + 1);
	if (out == NULL) {
		rc = 1;
		goto done;
	}
	strcpy(out, "#if !(defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)\n\ttypedef enum {\n\t\tfalse = 0,\n\t\ttrue = 1\n\t} bool;\n#endif\n");
	char* fin = NULL;

	//open the parse file
	char* parse_fn = (char*)malloc(strlen(args.in_file) + strlen(MMC_parsing_file_extension) + 1);
	if (parse_fn == NULL) {
		rc = 1;
		goto done;
	}
	strcpy(parse_fn, args.in_file);
	strcat(parse_fn, MMC_parsing_file_extension);

	fin = fget(parse_fn);

	struct {
		bool string : 1;
		bool c : 1;
		bool mut : 1;
	} flags = {false, false, false};

	size_t p = 0;
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

		//printf("<%s>", part);

		// #
		if (strcmp(part, "#") == 0) {
			if (memcmp(fin+p, "#startc", strlen("#startc")) == 0) {
				if (flags.c) {
					strcpy(err, "C block start while in C block!");
					rc = 2;
					goto done;
				}
				p += strlen("#startc");
				flags.c = true;
				continue;
			}
			else if (memcmp(fin+p, "#endc", strlen("#endc")) == 0) {
				if (!flags.c) {
					strcpy(err, "C block end where there was no C block!");
					rc = 2;
					goto done;
				}
				p += strlen("#endc");
				flags.c = false;
				continue;
			}
		}

		if (flags.c) {
			char* block = part;

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}

		else if (strcmp(part, "\n") == 0) {
			char* block = ";\n";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if ((strcmp(part, "\"") == 0) && (fin[p-1] != '\\') && (fin[p-1] != '\'')) {
			flags.string = !flags.string;
			char* block = part;

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (flags.string) {
			char* block = part;

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if ((strcmp(part, "(") == 0) || (strcmp(part, ")") == 0) || (strcmp(part, "|") == 0) || (strcmp(part, "*") == 0) ||
				(strcmp(part, "&") == 0) || (strcmp(part, "^") == 0) || (strcmp(part, "^") == 0) || (strcmp(part, "!") == 0) ||
				(strcmp(part, "%%") == 0) || (strcmp(part, "~") == 0) || (strcmp(part, "-") == 0) || (strcmp(part, "+") == 0) ||
				(strcmp(part, "=") == 0)
		){
			char* block = part;

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}

		// function def
		else if (strcmp(part, "func") == 0) {
		}

		// types
		else if (strcmp(part, "mut") == 0) {
			flags.mut = true;
		}
		// 8
		else if (strcmp(part, "u8") == 0) {
			char* block;
			if (flags.mut) {
				block = "unsigned char ";
				flags.mut = false;
			}
			else
				block = "const unsigned char ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "i8") == 0) {
			char* block;
			if (flags.mut) {
				block = "signed char ";
				flags.mut = false;
			}
			else
				block = "const signed char ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		// 16
		else if (strcmp(part, "u16") == 0) {
			char* block;
			if (flags.mut) {
				block = "unsigned short int ";
				flags.mut = false;
			}
			else
				block = "const unsigned short int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "i16") == 0) {
			char* block;
			if (flags.mut) {
				block = "signed short int ";
				flags.mut = false;
			}
			else
				block = "const signed short int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		// 32
		else if (strcmp(part, "u32") == 0) {
			char* block;
			if (flags.mut) {
				block = "unsigned int ";
				flags.mut = false;
			}
			else
				block = "const unsigned int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
			
		}
		else if (strcmp(part, "i32") == 0) {
			char* block;
			if (flags.mut) {
				block = "signed int ";
				flags.mut = false;
			}
			else
				block = "const signed int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		// 64
		else if (strcmp(part, "u64") == 0) {
			char* block;
			if (flags.mut) {
				block = "unsigned long long int ";
				flags.mut = false;
			}
			else
				block = "const unsigned long long int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
			
		}
		else if (strcmp(part, "i64") == 0) {
			char* block;
			if (flags.mut) {
				block = "signed long long int ";
				flags.mut = false;
			}
			else
				block = "const signed long long int ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "bool") == 0) {
			char* block;
			if (flags.mut) {
				block = "bool ";
				flags.mut = false;
			}
			else
				block = "const bool ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}

		// keywords
		else if (strcmp(part, "break") == 0) {
			char* block = "break ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "continue") == 0) {
			char* block = "continue ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "loop") == 0) {
			char* block = "while (true) ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "while") == 0) {
			char* block = "while ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "return") == 0) {
			char* block = "return ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "if") == 0) {
			char* block = "if ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "else") == 0) {
			char* block = "else ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}
		else if (strcmp(part, "NULL") == 0) {
			char* block = "NULL ";

			size_t l = strlen(out);

			char* mid = (char*)malloc(l + strlen(block) + 1);
			if (mid == NULL) {
				rc = 1;
				goto done;
			}

			if (l > 0)
				memcpy(mid, out, strlen(out));
			memcpy(mid + l, block, strlen(block) + 1);
			free(out);
			out = mid;
			mid = NULL;
		}

		p += strlen(part);
	}

	//printf("\n\n%s\n", out);

    done:
		if (parse_fn != NULL) {
			free(parse_fn);
			parse_fn = NULL;
		}
		if (fin != NULL) {
			free(fin);
			fin = NULL;
		}
		if (part != NULL) {
			free(part);
			part = NULL;
		}
		if (out != NULL) {
			free(out);
			out = NULL;
		}
		if (rc != 0) {
			error(err, true);
		}
		return rc;
}
