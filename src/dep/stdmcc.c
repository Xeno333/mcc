//stdmcc.c

//system
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//mcc
#include "stdmcc.h"


// Data:


const char license[] = "\tMIT License\
\t\
\tCopyright (c) 2024 Xeno333\
\t\
\tPermission is hereby granted, free of charge, to any person obtaining a copy\
\tof this software and associated documentation files (the \"Software\"), to deal\
\tin the Software without restriction, including without limitation the rights\
\tto use, copy, modify, merge, publish, distribute, sublicense, and/or sell\
\tcopies of the Software, and to permit persons to whom the Software is\
\tfurnished to do so, subject to the following conditions:\
\t\
\tThe above copyright notice and this permission notice shall be included in all\
\tcopies or substantial portions of the Software.\
\t\
\tTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\
\tIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\
\tFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\
\tAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\
\tLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\
\tOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\
\tSOFTWARE.\
\t";


//Credits
//Add name of person that contributes
const char* credits[] = {
	"Xeno333",
	"ENDOFLIST"
};


//Current compiling language
char* cur_lang = NULL;


//dev flag
bool dev_on = false;



// Array of string langs
const char lang_str_map[][STDMCC_str_map_str_len] = {
	"nosupport",
	"",//"c",
	"cesium",
	"",//"c++",
	"",//"fasm",
	"",//"go",
	"",//"rust",
	"",//"fortran",
	"",//"cobol",
	"",//"mitasm",
	"",//"midlang",
	"hrriscasm"
};

// Array of string archs
const char arch_str_map[][STDMCC_str_map_str_len] = {
	"noarch",
	"x86_64",
	"",//"x86",
	"hrrisc",
	"",//"arm64",
	"",//"arm",
	"",//"sparc",
	"csrc"
};

// Array of optimizations
const char opt_str_map[][STDMCC_str_map_str_len] = {
	"0",
	"1",
	"2",
	"3",
	"fast",
	"small"
};

// Array of formats
const char format_str_map[][STDMCC_str_map_str_len] = {
	"elf",
	"pe",
	"raw",
	"aout",
	"kqx",
	"obj",
	"o",
	"csrc"
};








// Code:



void error(const char* errors, bool self) {
	printf("\033[0;31m");
	if (self == true) {
		printself();
		printf("Error: ");
	}
	printf("%s", errors);
	printf("\033[0m");
}


//lib functions:
void list_supported_langs(void) {
	int n = sizeof(lang_str_map)/STDMCC_str_map_str_len;
	printf("Supported languages:\n");
	for (int j = 0; j < n; j++) {
		if (j == (lang_t)nosupport)
			continue;
		if (lang_str_map[j][0] != '\0') 
			printf("\t%s\n", lang_str_map[j]);
	}
}

void list_supported_formats(void) {
	int n = sizeof(format_str_map)/STDMCC_str_map_str_len;
	printf("Supported formats:\n");
	for (int j = 0; j < n; j++) {
		if (format_str_map[j][0] != '\0') 
			printf("\t%s\n", format_str_map[j]);
	}
}

void list_supported_archs(void) {
	int n = sizeof(arch_str_map)/STDMCC_str_map_str_len;
	printf("Supported architecturs:\n");
	for (int j = 0; j < n; j++) {
		if (j == (arch_t)noarch)
			continue;
		if (arch_str_map[j][0] != '\0') 
			printf("\t%s\n", arch_str_map[j]);
	}
}

void list_supported_opts(void) {
	int n = sizeof(opt_str_map)/STDMCC_str_map_str_len;
	printf("Supported optimzations:\n");
	for (int j = 0; j < n; j++) {
		if (opt_str_map[j][0] != '\0') 
			printf("\t%s\n", opt_str_map[j]);
	}
}

void list_credits(void) {
	int i = 0;
	printf("Credits:\n");
	while (strcmp(credits[i], "ENDOFLIST") != 0) {
		printf("\t%s\n", credits[i++]);
	}
}

void print_license(void) {
	printf("%s", license);
}

void help(void) {
	printself();
	printf("Usage: mcc (request) <lang> <options>\n");
	printf("\
		Request:\n\
			?=help\tDisplay this message.\n\
			?=archs\tList supported architecturs.\n\
			?=formats\tList formats.\n\
			?=opts\tList supported optimzations.\n\
			?=langs\tList supported languages.\n\
			?=credits\tList credits.\n\
			?=license\tPrint license.\n\
			?=man\tShow man file for languge specified in '-lang=<lang>'\n\n\
		Options:\n\
			-src=<src file>\n\
			-out=<outfile>\n\
			-arch=<arch>\n\
			-opt=<opt level>\n\
			-std=<std ID>\n\
			-format=<outputed format>\n\
			-inc=<path to include from>\n\
			-dev=on\tEnable development features.\n\
			-lang=<language>\
	\n");

	printf("\n");
	list_supported_opts();
	printf("\n");
	list_supported_archs();
	printf("\n");
	list_supported_formats();
	printf("\n");
	list_supported_langs();
}



int fsize(const char* fn) {
	FILE* f = fopen(fn, "rb");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fclose(f);
	return size;
}

char* fget(const char* fn) {
	FILE* f = fopen(fn, "rb");
	if (f == NULL) {
		return NULL;
	}
	int size = fsize(fn);
	char* block = malloc(size+1);
	size_t r = fread(block, 1, size, f);
	if (r < size) {
		if (ferror(f)) {
			free(block);
			return NULL;
		}
	}
	fclose(f);
	block[size+1] = 0;
	return block;
}

static const char* getmanpath() {
	return manpath;
}

int man(const char* manfn) {
	char* fn = malloc(strlen(manfn) + strlen(getmanpath()) + sizeof(char));
	strcpy(fn, getmanpath());
	strcat(fn, manfn);
	char* f = fget(fn);
	if (f == NULL) {
		error("Could not open requested man file!\n", true);
		free(fn);
		return -1;
	}
	free(fn);

	printf("%s\n", f);

	free(f);
	f = NULL;
	return 0;
}


//Print compiler anme and version in formatted way
void printself(void) {
	printf("<%s@%s:%s> ", MCCName, MCCVersion, cur_lang);
}




//Get arg from args format: <argtype><arg>
char* get_chara_arg(const int argc, const char** args, const char* argtype) {
	//find right flag
	for (int i = 0; i < argc; i++) {
		if (strncmp(args[i], argtype, strlen(argtype)) == 0) {
			const char* str = args[i] + strlen(argtype);
			return (char*)str;
		}
	}
	return NULL;
}

char* get_args(compiler_args_t* argout, const int argc,const char** args) {
	// Get lang type
	int n = sizeof(lang_str_map)/STDMCC_str_map_str_len;
	const char* lang = get_chara_arg(argc, args, "-lang=");
	if (lang != NULL) {
		bool found = false;
		for (int j = 0; j < n; j++) {
			if (strcmp(lang, lang_str_map[j]) == 0) {
				argout->lang = (lang_t)j;
				found = true;
				break;
			}
		}
		if (found == false)
			return "Language invalid.";
		if (argout->lang == nosupport)
			return "Language invalid.";
		else if (lang_str_map[argout->lang][0] == '\0')
			return "Language invalid.";
	}
	else 
		return "Language not specified.";
	

	// Get arch type
	n = sizeof(arch_str_map)/STDMCC_str_map_str_len;
	const char* arch = get_chara_arg(argc, args, "-arch=");
	if (arch != NULL) {
		bool found = false;
		for (int j = 0; j < n; j++) {
			if (strcmp(arch, arch_str_map[j]) == 0) {
				argout->arch = (arch_t)j;
				found = true;
				break;
			}
		}
		if (found == false)
			return "Architecture invalid.";
		else if (argout->arch == noarch)
			return "Architecture invalid.";
		else if (arch_str_map[argout->arch][0] == '\0')
			return "Architecture invalid.";
	}
	else 
		return "Architecture not specified.";

	// Get op_lvl
	n = sizeof(opt_str_map)/STDMCC_str_map_str_len;
	argout->op_lvl = O0;
	const char* opt = get_chara_arg(argc, args, "-opt=");
	if (opt != NULL) {
		bool found = false;
		for (int j = 0; j < n; j++) {
			if (strcmp(opt, opt_str_map[j]) == 0) {
				argout->op_lvl = (op_lvl_t)j;
				found = true;
				break;
			}
		}
		if (found == false)
			return "Optimzation invalid.";
		if (opt_str_map[argout->op_lvl][0] == '\0')
			return "Optimzation invalid.";
	}
		

	// Get format type
	n = sizeof(format_str_map)/STDMCC_str_map_str_len;
	const char* format = get_chara_arg(argc, args, "-format=");
	if (format != NULL) {
		bool found = false;
		for (int j = 0; j < n; j++) {
			if (strcmp(format, format_str_map[j]) == 0) {
				argout->format = (format_t)j;
				found = true;
				break;
			}
		}
		if (found == false)
			return "Format invalid.";
		else if (format_str_map[argout->format][0] == '\0')
			return "Format invalid.";
	}
	else 
		return "Format not specified.";


	//char*
	argout->in_file = get_chara_arg(argc, args, "-src=");
	argout->out_file = get_chara_arg(argc, args, "-out=");
	argout->std = get_chara_arg(argc, args, "-std=");
	argout->inc_path = get_chara_arg(argc, args, "-inc=");

	return NULL;
}
