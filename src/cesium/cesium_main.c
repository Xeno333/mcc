//cesium_main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//mcc
#include "stdmcc.h"
//cesium
#include "cesium.h"



int cesium_main(const compiler_args_t args) {
	cur_lang = "cesium";

	if ((args.std != NULL) && (strcmp(args.std, "alpha") != 0)) {
		error("Invalid std specified: ", true);
		error(args.std, false);
		printf("\n");
	}
	if (args.arch != csrc) {
		error("Invalid architecture specified: ", true);
		error(arch_str_map[args.arch], false);
		printf("\n");
	}

	if (parse_file(args) != 0) {
		error("An Error occored while attempting to parse! complination stoping!\n", true);
		return -1;
	}

	return compile_file(args);

	error("Cesium not implemented yet!\n", true);

	return -1;
}
