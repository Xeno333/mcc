//main.c

//system
#include <stdio.h>
#include <string.h>
//mcc
#include "stdmcc.h"





//main hook for project
int main(const int argc,const char** args) {
	if (argc < 2) {
		printself();
		printf("Use 'mcc ?=help' for help.\n");
		return -1;
	}

	char* request = get_chara_arg(argc, args, "?=");

	// check dev flag
	char* dev = get_chara_arg(argc, args, "-dev=");
	if ((dev != NULL) && (strcmp(dev, "on") == 0)) 
		dev_on = true;

	if (request != NULL) {
		if ((strcmp(request, "help") == 0) && (!(argc > 2))) {
			help();
			return 0;
		}
		else if ((strcmp(request, "langs") == 0) && (!(argc > 2))) {
			printself();
			list_supported_langs();
			return 0;
		}
		else if ((strcmp(request, "formats") == 0) && (!(argc > 2))) {
			printself();
			list_supported_formats();
			return 0;
		}
		else if ((strcmp(request, "archs") == 0) && (!(argc > 2))) {
			printself();
			list_supported_archs();
			return 0;
		}
		else if ((strcmp(request, "opts") == 0) && (!(argc > 2))) {
			printself();
			list_supported_opts();
			return 0;
		}
		else if ((strcmp(request, "credits") == 0) && (!(argc > 2))) {
			printself();
			list_credits();
			return 0;
		}
		else if ((strcmp(request, "license") == 0) && (!(argc > 2))) {
			printself();
			print_license();
			return 0;
		}
	else if ((strcmp(request, "man") == 0) && (!(argc > 3))) {
			char* lang = get_chara_arg(argc, args, "-lang=");
		if (lang == NULL) {
			error("Invalid attempt to get man file!\n", true);
			return -1;
		}
			return man(lang);
	}
	else {
		error("Number of args is wrong!\n", true);
		return -1;
	}
	}

	compiler_args_t arg;
	char* rets = get_args(&arg, argc, args);
	if (rets != NULL) {
		error(rets, true);
		printf("\nUse 'mcc ?=help' for help.\n");
		return -1;
	}

	if (arg.in_file == NULL) {
		error("No src file!\n", true);
		printf("Use 'mcc ?=help' for help.\n");
		return -1;
	}
	if (arg.out_file == NULL) {
		error("No out file!\n", true);
		printf("Use 'mcc ?=help' for help.\n");
		return -1;
	}

	/*
	Compiler may close on error free all ram
	*/


	//modual return value
	int retv = 0;

	bool done = false;


	//dev languages
	if (dev_on == true) {
		if (arg.lang == cesium) {
			retv = cesium_main(arg);
			done = true;
		}
	}

	// hrriscasm
	if (arg.lang == hrriscasm && arg.arch != hrrisc) {
		error("Pre-compiler: hriscasm requires '-arch=hrrisc'\n", true);
		retv = -1;
		done = true;
	}
	if (arg.lang == hrriscasm && arg.arch == hrrisc) {
		retv = hrriscasm_main(arg);
		done = true;
	}

	if (done == false) {
		error("Language '", true);
		error(lang_str_map[arg.lang], false);
		error("' is not suported! Maybe try with '-dev=on' flag\n", false);
	}


	return retv;
}
