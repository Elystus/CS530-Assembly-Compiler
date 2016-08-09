/*  Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog4
    CS530, Spring 2016
 */

#ifndef SICXE_ASM_H
#define SICXE_ASM_H

#include <string>
#include "file_parser.h"
#include "opcodetab.h"
#include "symtab.h"

using namespace std;

class sicxe_asm {
	public:
		sicxe_asm(string);
		~sicxe_asm();
	private: 
		// Struct for listing file	
		struct first_pass_row {
			int line;
			string addr;
			string label;
			string opcode;
			string operand;
		};
		
		struct second_pass_row {
			int line;
			string addr;
			string label;
			string opcode;
			string operand;
			string machine;
		};
		
		opcodetab opcode_index;
		symtab symbol_table;
		vector<first_pass_row> first_pass_vector;
		vector<second_pass_row> second_pass_vector;
		vector<string> equ_mod_fail;
		string filename;
		unsigned int base_address;
		bool set_base;
		
		// Variables for Object code generator
		unsigned int program_length;
		unsigned int program_start;
		string program_name;
		
		// First Pass
		void first_pass(file_parser);
		bool directive_check(string);
		unsigned int directive_helper(string, string, string, unsigned int);
		unsigned int directive_helper_start(string, unsigned int);
		unsigned int directive_helper_byte(string, unsigned int);
		unsigned int directive_helper_resb(string, unsigned int);
		unsigned int directive_helper_resw(string, unsigned int);
		void directive_helper_equ(string, string, unsigned int);
		void write_to_file();
		
		// Second Pass
		void second_pass();
		unsigned int nixbpe_generator(bool, bool, bool, bool, bool, bool, first_pass_row);
		string directive_helper_sp(first_pass_row);
		void directive_helper_sp_base(first_pass_row);
		string directive_helper_sp_byte(first_pass_row);
		string directive_helper_sp_word(first_pass_row);
		string helper_sp_format1(first_pass_row);
		string helper_sp_format2(first_pass_row);
		string helper_sp_format3(first_pass_row);
		string helper_sp_format4(first_pass_row);

		// Object Code Generator
		void generate_obj();
		
		// Offset Calculations/Checks
		int relative_pc(first_pass_row, unsigned int);
		bool relative_pc_check(int);
		unsigned int relative_base(first_pass_row, unsigned int);
		bool relative_base_check(unsigned int);
		
		// SYMTAB helper function
		unsigned int get_label(string, first_pass_row);

		// Helper Functions
		string hex_to_string(unsigned int);
		string hex_to_string_nofill(unsigned int);
		string hex_to_string(unsigned int, unsigned int);
		string hex_to_string(int, unsigned int);
		string int_to_string(unsigned int);
		unsigned int string_to_hex(string);
		unsigned int string_to_int(string);
		string get_register(string, first_pass_row);
		string to_upper(string);
		bool is_num(string);
		bool is_hex(string);

		// Error Helper
		void throw_error(string);
		void throw_error(string, unsigned int);
};
#endif
