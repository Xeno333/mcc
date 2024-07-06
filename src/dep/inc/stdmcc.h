//stdmcc.h

#ifndef stdmcc
#define stdmcc

//Compiler defs
	#define MCCVersion "v0.0.1"
	#define MCCName "mcc"//"MIT licesnsed Compiler Collection"
	#define manpath "./man/"

	#define MMC_parsing_file_extension ".mccpf" // mcc pars file

//lib spesific defs
	#define STDMCC_str_map_str_len 256


//Types
	#if !(defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
		typedef enum {
			false = 0,
			true = 1
		} bool;
	#endif

	typedef enum {
		nosupport = 0,
		c = 1, // c
		cesium = 2, // cesium
		cpp = 3, // c++
		fasm = 4, // fasm
		go = 5, // go
		rust = 6, // rust
		fortran = 7, // fortran
		cobol = 8, // cobol
		mitasm = 9, // mit licesned asm, need to make
		midlang = 10, // for midlevel arch-independent, for compilation like llvm
		hrriscasm = 11 // HR-RISC asm
	} lang_t;

	typedef enum {
		O0 = 0,
		O1 = 1,
		O2 = 2,
		O3 = 3,
		Ofast = 4,
		Osmall = 5,
	} op_lvl_t;

	typedef enum {
		elf = 0,
		pe = 1,
		raw = 2,
		aout = 3,
		kqx = 4, // Kumquat exe
		obj = 5,
		o = 6,
		format_csrc = 7 // Compile to C
	} format_t;

	typedef enum {
		noarch = 0,
		x86_64 = 1,
		x86 = 2,
		hrrisc = 3,
		arm64 = 4,
		arm = 5,
		sparc = 6,
		csrc = 7 // Compile to C
	} arch_t;

	//args struct
	typedef struct {
		lang_t lang;
		char* in_file;
		char* out_file;
		op_lvl_t op_lvl;
		format_t format;
		char* std; // compiler modoule spesific
		arch_t arch; // output arch
		//add linker info later when ready
		char* inc_path;
	} compiler_args_t;



//Modual helper types
	//
	typedef struct {
		char* str;
		void* next;
	} str_linked_list_t;





//Lib defined
	//print error
	void error(const char* errors, bool self);
	char* get_chara_arg(const int argc, const char** args, const char* argtype);
	// gets args
	char* get_args(compiler_args_t* argout, const int argc,const char** args);
	// print mcc <name @ version>
	void printself(void);
	// list_supported_langs
	void list_supported_langs(void);
	// list_supported_formats
	void list_supported_formats(void);
	// list_supported_archs
	void list_supported_archs(void);
	// list_supported_opts
	void list_supported_opts(void);
	// List credits
	void list_credits(void);
	void print_license(void);
	//help
	void help(void);
	int fsize(const char* fn);
	char* fget(const char* fn);
	int man(const char* manfn);
	bool contains(char* str, const char c);


//lib vars
	extern const char lang_str_map[][STDMCC_str_map_str_len];
	extern const char arch_str_map[][STDMCC_str_map_str_len];
	extern const char opt_str_map[][STDMCC_str_map_str_len];
	extern char* cur_lang;
	extern bool dev_on;


//Lang compiler hooks
	int cesium_main(const compiler_args_t args);
	int hrriscasm_main(const compiler_args_t args);


#endif  // stdmcc
